#include "executor.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
namespace fgl {
using namespace framegraph;
Dx12PassContext::Dx12PassContext(ID3D12GraphicsCommandList* list, const CompiledPass& pass, Dx12PlacedResourceArena& arena, const ReadbackLayout& layout, std::uint32_t frame)
    : list_(list), pass_(pass), arena_(arena), layout_(layout), logical_frame_(frame) {}
const ResourceUsage& Dx12PassContext::declared(ResourceId id) const {
    for (const auto& use : pass_.usages) if (use.resource == id) return use;
    throw GpuFailure("UndeclaredAccess", "pass " + std::to_string(pass_.id.value) + " accessed undeclared resource " + std::to_string(id.value));
}
void Dx12PassContext::require(ResourceId id, Usage usage) const {
    if (declared(id).usage != usage) throw GpuFailure("UndeclaredUsage", "callback operation conflicts with declared usage");
}
ID3D12Resource* Dx12PassContext::resource(ResourceId id) const { (void)declared(id); return arena_.resource(id); }
D3D12_CPU_DESCRIPTOR_HANDLE Dx12PassContext::rtv(ResourceId id) const { require(id, Usage::RenderTarget); return arena_.rtv(id); }
D3D12_CPU_DESCRIPTOR_HANDLE Dx12PassContext::dsv(ResourceId id, bool read_only) const { require(id, read_only ? Usage::DepthRead : Usage::DepthWrite); return arena_.dsv(id, read_only); }
D3D12_GPU_DESCRIPTOR_HANDLE Dx12PassContext::srv(ResourceId id) const { require(id, Usage::ShaderRead); return arena_.srv(id); }
void Dx12PassContext::clear_rtv(ResourceId id, const std::array<float, 4>& color) { list_->ClearRenderTargetView(rtv(id), color.data(), 0, nullptr); }
void Dx12PassContext::clear_uav(ResourceId id, const std::array<std::uint32_t, 4>& values) {
    require(id, Usage::UnorderedAccess);
    list_->ClearUnorderedAccessViewUint(arena_.uav_gpu(id), arena_.uav_cpu(id), resource(id), values.data(), 0, nullptr);
}
void Dx12PassContext::clear_uav_float(ResourceId id, const std::array<float, 4>& values) {
    require(id, Usage::UnorderedAccess);
    list_->ClearUnorderedAccessViewFloat(arena_.uav_gpu(id), arena_.uav_cpu(id), resource(id), values.data(), 0, nullptr);
}
void Dx12PassContext::copy(ResourceId source, ResourceId destination) {
    require(source, Usage::CopySource); require(destination, Usage::CopyDest); list_->CopyResource(resource(destination), resource(source));
}
void Dx12PassContext::readback(ResourceId source, ResourceId destination) {
    require(source, Usage::CopySource); require(destination, Usage::CopyDest);
    copy_to_readback(list_, resource(source), resource(destination), layout_);
}
void Dx12PassContext::copy_buffer(ResourceId source, ResourceId destination, std::uint64_t bytes) {
    require(source, Usage::CopySource);
    require(destination, Usage::CopyDest);
    const auto source_desc = resource(source)->GetDesc();
    const auto destination_desc = resource(destination)->GetDesc();
    if (!bytes || source_desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER || destination_desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER
        || bytes > source_desc.Width || bytes > destination_desc.Width)
        throw GpuFailure("InvalidBufferCopy", "buffer copy is empty, out of bounds, or targets a texture");
    list_->CopyBufferRegion(resource(destination), 0, resource(source), 0, bytes);
}
D3D12_GPU_DESCRIPTOR_HANDLE Dx12PassContext::uav(ResourceId id) const { require(id, Usage::UnorderedAccess); return arena_.uav_gpu(id); }
void Dx12PassContext::indirect(ID3D12CommandSignature* signature, ResourceId arguments) {
    require(arguments, Usage::IndirectArgument);
    if (!signature) throw GpuFailure("InvalidIndirect", "command signature is null");
    list_->ExecuteIndirect(signature, 1, resource(arguments), 0, nullptr, 0);
}
struct Dx12GraphExecutor::FrameData {
    Dx12PlacedResourceArena arena;
    ComPtr<ID3D12Resource> pixels, timestamps, cull_readback;
    ComPtr<ID3D12QueryHeap> query_heap;
    bool pending{};
    FrameData(ID3D12Device* device, const CompiledPlan& plan, const ReadbackLayout& layout)
        : arena(device, plan), pixels(readback_buffer(device, layout.bytes)), timestamps(readback_buffer(device, std::max<std::uint64_t>(8, plan.graph.passes.size() * 16))) {
        D3D12_QUERY_HEAP_DESC desc{}; desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP; desc.Count = std::max(1u, static_cast<UINT>(plan.graph.passes.size() * 2));
        check_hr(device->CreateQueryHeap(&desc, IID_PPV_ARGS(&query_heap)), "CreateQueryHeap", device);
    }
};
Dx12GraphExecutor::Dx12GraphExecutor(Dx12Context& context, GraphDescription graph, std::vector<PassCallback> callbacks,
    ResourceId backbuffer, ResourceId readback, RuntimeReport& report, bool aliasing)
    : Dx12GraphExecutor(context, std::move(graph), std::move(callbacks), backbuffer, readback, ResourceId{}, report, aliasing) {}
Dx12GraphExecutor::Dx12GraphExecutor(Dx12Context& context, GraphDescription graph, std::vector<PassCallback> callbacks,
    ResourceId backbuffer, ResourceId readback, ResourceId cull_readback, RuntimeReport& report, bool aliasing)
    : context_(context), report_(report), callbacks_(std::move(callbacks)), backbuffer_(backbuffer), readback_(readback), cull_readback_(cull_readback), layout_(readback_layout(context.device(), context.width(), context.height())) {
    const auto start = std::chrono::steady_clock::now();
    // Validate before any native descriptor query. All decisions below come from
    // the exact same Core compiler/planners used by PlanCompiler and its tests.
    auto compiled = GraphCompiler::compile(graph);
    if (!compiled) throw GpuFailure("InvalidGraph", compiled.error().message);
    const auto requirements = device_requirements(context.device(), compiled->description);
    auto allocation = TransientAllocator::plan(*compiled, requirements, aliasing);
    if (!allocation) throw GpuFailure("InvalidAllocation", allocation.error().message);
    auto barriers = ResourceStatePlanner::plan(*compiled, *allocation);
    if (!barriers) throw GpuFailure("InvalidBarrierPlan", barriers.error().message);
    plan_ = {std::move(*compiled), std::move(*allocation), std::move(*barriers)};
    ++report_.plan_compile_count;
    report_.compile_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    if (callbacks_.size() != graph.passes.size()) throw GpuFailure("MissingCallback", "callback count differs from declaration count");
    for (const auto& pass : plan_.graph.passes) if (!callbacks_[pass.id.value]) throw GpuFailure("MissingCallback", "active pass has no callback");
    check_hr(context.queue()->GetTimestampFrequency(&frequency_), "GetTimestampFrequency", context.device());
    if (!frequency_) throw GpuFailure("TimestampFrequency", "GPU timestamp frequency is zero");
    report_.gpu_timestamp_frequency = frequency_;
    report_.placed_resource_count = 0;
    for (auto& frame : frames_) {
        frame = std::make_unique<FrameData>(context.device(), plan_, layout_);
        if (cull_readback_.value != invalid_index)
            frame->cull_readback = readback_buffer(context.device(), sizeof(D3D12_DRAW_ARGUMENTS));
        report_.placed_resource_count += frame->arena.placed_count();
    }
    report_.planned_heap_bytes = plan_.allocation.physical_bytes;
    report_.actual_heap_bytes = frames_[0]->arena.actual_heap_bytes();
    report_.all_frame_heap_bytes = 0;
    for (const auto& frame : frames_) {
        if (frame->arena.actual_heap_bytes() != report_.planned_heap_bytes) throw GpuFailure("HeapPlanMismatch", "actual per-frame arena differs from Core plan");
        report_.all_frame_heap_bytes += frame->arena.actual_heap_bytes();
    }
    report_.committed_bytes = plan_.allocation.committed_bytes; report_.saved_bytes = plan_.allocation.saved_bytes;
    report_.heap_count = static_cast<std::uint32_t>(plan_.allocation.heaps.size());
    report_.alias_reuse_events = static_cast<std::uint32_t>(plan_.allocation.aliases.size());
    report_.logical_bytes = 0;
    for (std::size_t i = 0; i < graph.resources.size(); ++i) if (plan_.allocation.resources[i]) {
        if (const auto* t = std::get_if<TextureDesc>(&graph.resources[i].descriptor)) report_.logical_bytes += static_cast<std::uint64_t>(t->width) * t->height * (t->format == Format::Rgba16Float ? 8u : 4u);
        else report_.logical_bytes += std::get<BufferDesc>(graph.resources[i].descriptor).bytes;
    }
    report_.plan_identity = framegraph::plan_identity(plan_); report_.current_graph = report_.plan_identity;
    report_.transition_count = plan_.barriers.transition_count; report_.uav_count = plan_.barriers.uav_count; report_.aliasing_count = plan_.barriers.aliasing_count;
    report_.pass_timings.clear();
    for (const auto& pass : plan_.graph.passes) report_.pass_timings.push_back(PassTiming{graph.passes[pass.id.value].name});
}
Dx12GraphExecutor::~Dx12GraphExecutor() { try { context_.wait_idle(); } catch (...) {} }
void Dx12GraphExecutor::emit(ID3D12GraphicsCommandList* list, Dx12PlacedResourceArena& arena, const std::vector<Barrier>& barriers) {
    for (const auto& source : barriers) {
        D3D12_RESOURCE_BARRIER barrier{};
        switch (source.kind) {
        case BarrierKind::Transition:
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition = {arena.resource(source.resource), D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, native_state(source.before), native_state(source.after)};
            ++report_.executed_transitions; break;
        case BarrierKind::Uav:
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV; barrier.UAV.pResource = arena.resource(source.resource); ++report_.executed_uav_barriers; break;
        case BarrierKind::Aliasing:
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
            barrier.Aliasing.pResourceBefore = source.native_alias_before_null || source.alias_before.value == invalid_index ? nullptr : arena.resource(source.alias_before);
            barrier.Aliasing.pResourceAfter = arena.resource(source.resource); ++report_.executed_alias_barriers; break;
        }
        list->ResourceBarrier(1, &barrier);
    }
}
void Dx12GraphExecutor::record(std::uint32_t logical_frame) {
    const auto index = context_.frame_index();
    collect(index); // begin_frame already waited the owning frame fence.
    auto& frame = *frames_[index];
    frame.arena.bind_import(backbuffer_, context_.backbuffer()); frame.arena.bind_import(readback_, frame.pixels.Get());
    if (cull_readback_.value != invalid_index) frame.arena.bind_import(cull_readback_, frame.cull_readback.Get());
    auto* list = context_.current_list();
    ID3D12DescriptorHeap* heaps[]{frame.arena.shader_heap()}; list->SetDescriptorHeaps(1, heaps);
    for (std::uint32_t i = 0; i < plan_.graph.passes.size(); ++i) {
        const auto& pass = plan_.graph.passes[i]; context_.current_pass = plan_.graph.description.passes[pass.id.value].name;
        report_.current_pass = context_.current_pass;
        const auto start = std::chrono::steady_clock::now();
        list->EndQuery(frame.query_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, i * 2);
        emit(list, frame.arena, plan_.barriers.passes[i].before);
        for (const auto& barrier : plan_.barriers.passes[i].before) if (barrier.kind == BarrierKind::Aliasing) {
            for (const auto& use : pass.usages) if (use.resource == barrier.resource) {
                if (use.usage == Usage::RenderTarget) {
                    const float clear[]{0.f, 0.f, 0.f, 1.f};
                    list->ClearRenderTargetView(frame.arena.rtv(use.resource), clear, 0, nullptr);
                } else if (use.usage == Usage::DepthWrite) {
                    list->ClearDepthStencilView(frame.arena.dsv(use.resource, false), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
                }
            }
        }
        Dx12PassContext pass_context(list, pass, frame.arena, layout_, logical_frame);
        callbacks_[pass.id.value](pass_context);
        emit(list, frame.arena, plan_.barriers.passes[i].after);
        list->EndQuery(frame.query_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, i * 2 + 1);
        const double elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        auto& timing = report_.pass_timings[i];
        timing.cpu_record_ms = (timing.cpu_record_ms * static_cast<double>(timing.cpu_samples) + elapsed) / static_cast<double>(timing.cpu_samples + 1); ++timing.cpu_samples;
    }
    emit(list, frame.arena, plan_.barriers.final);
    if (!plan_.graph.passes.empty()) list->ResolveQueryData(frame.query_heap.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0, static_cast<UINT>(plan_.graph.passes.size() * 2), frame.timestamps.Get(), 0);
}
void Dx12GraphExecutor::submitted() { last_frame_ = context_.frame_index(); frames_[last_frame_]->pending = true; has_frame_ = true; }
void Dx12GraphExecutor::collect(std::uint32_t index) {
    auto& frame = *frames_[index]; if (!frame.pending) return;
    const auto count = plan_.graph.passes.size(); const D3D12_RANGE range{0, count * 16};
    void* mapped{}; check_hr(frame.timestamps->Map(0, &range, &mapped), "Map timestamps", context_.device());
    std::vector<std::uint64_t> ticks(count * 2); std::memcpy(ticks.data(), mapped, count * 16);
    const D3D12_RANGE written{0, 0}; frame.timestamps->Unmap(0, &written);
    for (std::size_t i = 0; i < count; ++i) {
        if (ticks[i * 2 + 1] < ticks[i * 2]) throw GpuFailure("TimestampOrder", "GPU timestamp pair is reversed");
        const double elapsed = static_cast<double>(ticks[i * 2 + 1] - ticks[i * 2]) * 1000.0 / static_cast<double>(frequency_);
        auto& timing = report_.pass_timings[i]; timing.gpu_ms = (timing.gpu_ms * static_cast<double>(timing.gpu_samples) + elapsed) / static_cast<double>(timing.gpu_samples + 1); ++timing.gpu_samples;
    }
    frame.pending = false;
}
std::vector<std::uint8_t> Dx12GraphExecutor::finish(std::uint32_t timeout_ms) {
    if (!has_frame_) throw GpuFailure("NoFrame", "no completed frame is available for readback");
    if (!timeout_ms) throw GpuFailure("CaptureTimeout", "capture deadline is zero");
    context_.wait(context_.frame_fence(last_frame_), timeout_ms);
    context_.wait_idle();
    for (std::uint32_t i = 0; i < frame_count; ++i) collect(i);
    if (cull_readback_.value != invalid_index) {
        const D3D12_RANGE range{0, sizeof(D3D12_DRAW_ARGUMENTS)};
        void* mapped{};
        check_hr(frames_[last_frame_]->cull_readback->Map(0, &range, &mapped), "Map culling readback", context_.device());
        D3D12_DRAW_ARGUMENTS args{};
        std::memcpy(&args, mapped, sizeof(args));
        const D3D12_RANGE written{0, 0};
        frames_[last_frame_]->cull_readback->Unmap(0, &written);
        report_.gpu_visible_count = args.InstanceCount;
        if (report_.gpu_visible_count != report_.cpu_visible_count)
            throw GpuFailure("CullingParity", "GPU/CPU visible instance counts differ");
    }
    return read_rgba(frames_[last_frame_]->pixels.Get(), layout_);
}
}
