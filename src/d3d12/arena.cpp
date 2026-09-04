#include "arena.hpp"
#include <algorithm>
namespace fgl {
using namespace framegraph;
D3D12_RESOURCE_DESC native_descriptor(const ResourceDescription& resource) {
    D3D12_RESOURCE_DESC desc{}; desc.DepthOrArraySize = 1; desc.MipLevels = 1; desc.SampleDesc.Count = 1;
    if (const auto* texture = std::get_if<TextureDesc>(&resource.descriptor)) {
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; desc.Width = texture->width; desc.Height = texture->height;
        switch (texture->format) {
        case Format::Rgba8: desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
        case Format::Rgba16Float: desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
        case Format::D32Float: desc.Format = DXGI_FORMAT_R32_TYPELESS; break;
        }
        if (texture->render_target) desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        if (texture->depth_stencil) desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        if (texture->unordered_access) desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    } else {
        const auto& buffer = std::get<BufferDesc>(resource.descriptor);
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; desc.Width = buffer.bytes; desc.Height = 1; desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        if (buffer.unordered_access) desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }
    return desc;
}
std::vector<MemoryRequirement> device_requirements(ID3D12Device* device, const GraphDescription& graph) {
    std::vector<MemoryRequirement> requirements;
    for (const auto& resource : graph.resources) {
        if (resource.imported) { requirements.push_back({}); continue; }
        const auto desc = native_descriptor(resource);
        const auto actual = device->GetResourceAllocationInfo(0, 1, &desc);
        if (!actual.SizeInBytes || actual.SizeInBytes == UINT64_MAX || actual.Alignment != D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
            throw GpuFailure("AllocationRequirement", "device does not support requested default 64-KiB resource placement: " + resource.name);
        const auto type = desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER ? HeapClass::Buffer
            : (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) ? HeapClass::RtDsTexture : HeapClass::Texture;
        requirements.push_back({actual.SizeInBytes, actual.Alignment, type, 0, false});
    }
    return requirements;
}
D3D12_RESOURCE_STATES native_state(ResourceState state) {
    switch (state) {
    case ResourceState::Common: return D3D12_RESOURCE_STATE_COMMON;
    case ResourceState::Present: return D3D12_RESOURCE_STATE_PRESENT;
    case ResourceState::ShaderRead: return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    case ResourceState::RenderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case ResourceState::DepthWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case ResourceState::DepthRead: return D3D12_RESOURCE_STATE_DEPTH_READ;
    case ResourceState::UnorderedAccess: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case ResourceState::CopySource: return D3D12_RESOURCE_STATE_COPY_SOURCE;
    case ResourceState::CopyDest: return D3D12_RESOURCE_STATE_COPY_DEST;
    case ResourceState::IndirectArgument: return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    }
    throw GpuFailure("InvalidPlanState", "unmapped Core state");
}
DescriptorHeap::DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, std::uint32_t capacity, bool visible)
    : capacity_(std::max(1u, capacity)), stride_(device->GetDescriptorHandleIncrementSize(type)), visible_(visible) {
    D3D12_DESCRIPTOR_HEAP_DESC desc{}; desc.Type = type; desc.NumDescriptors = capacity_; desc.Flags = visible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    check_hr(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap_)), "CreateDescriptorHeap(arena)", device);
}
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::cpu(std::uint32_t index) const {
    if (index >= capacity_) throw GpuFailure("DescriptorExhausted", "CPU descriptor index exceeds owned heap");
    auto handle = heap_->GetCPUDescriptorHandleForHeapStart(); handle.ptr += static_cast<SIZE_T>(index) * stride_; return handle;
}
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::gpu(std::uint32_t index) const {
    if (!visible_ || index >= capacity_) throw GpuFailure("DescriptorExhausted", "GPU descriptor index exceeds owned visible heap");
    auto handle = heap_->GetGPUDescriptorHandleForHeapStart(); handle.ptr += static_cast<UINT64>(index) * stride_; return handle;
}
Dx12PlacedResourceArena::Dx12PlacedResourceArena(ID3D12Device* device, const CompiledPlan& plan)
    : device_(device), plan_(plan), count_(static_cast<std::uint32_t>(plan.graph.description.resources.size())), owned_(count_), resources_(count_),
      rtv_(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, count_), dsv_(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, count_ * 2),
      cpu_(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, count_ * 2), gpu_(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, count_ * 2, true) {
    for (const auto& physical : plan.allocation.heaps) {
        D3D12_HEAP_DESC desc{}; desc.SizeInBytes = physical.bytes; desc.Alignment = physical.alignment;
        desc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT; desc.Properties.CreationNodeMask = desc.Properties.VisibleNodeMask = 1;
        switch (physical.heap_class) {
        case HeapClass::Buffer: desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS; break;
        case HeapClass::RtDsTexture: desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES; break;
        case HeapClass::Texture: desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES; break;
        }
        ComPtr<ID3D12Heap> heap; check_hr(device->CreateHeap(&desc, IID_PPV_ARGS(&heap)), "CreateHeap", device);
        const auto actual = heap->GetDesc();
        if (actual.SizeInBytes != physical.bytes || actual.Alignment != physical.alignment) throw GpuFailure("HeapPlanMismatch", "created heap differs from Core plan");
        heap_bytes_ += actual.SizeInBytes; heaps_.push_back(std::move(heap));
    }
    for (std::uint32_t i = 0; i < count_; ++i) {
        if (!plan.allocation.resources[i]) continue;
        const auto& allocation = *plan.allocation.resources[i];
        const auto desc = native_descriptor(plan.graph.description.resources[i]);
        D3D12_CLEAR_VALUE optimized{};
        const D3D12_CLEAR_VALUE* optimized_ptr = nullptr;
        if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {
            optimized.Format = desc.Format; optimized.Color[3] = 1.f; optimized_ptr = &optimized;
        } else if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
            optimized.Format = DXGI_FORMAT_D32_FLOAT; optimized.DepthStencil.Depth = 1.f; optimized_ptr = &optimized;
        }
        check_hr(device->CreatePlacedResource(heaps_[allocation.heap].Get(), allocation.offset, &desc, D3D12_RESOURCE_STATE_COMMON, optimized_ptr, IID_PPV_ARGS(&owned_[i])), "CreatePlacedResource", device);
        resources_[i] = owned_[i].Get(); ++placed_count_; create_views(ResourceId{i});
    }
}
void Dx12PlacedResourceArena::bind_import(ResourceId id, ID3D12Resource* resource) {
    if (id.value >= count_ || !plan_.graph.description.resources[id.value].imported || !resource) throw GpuFailure("InvalidImport", "invalid imported resource binding");
    resources_[id.value] = resource; create_views(id);
}
ID3D12Resource* Dx12PlacedResourceArena::resource(ResourceId id) const {
    if (id.value >= count_ || !resources_[id.value]) throw GpuFailure("InvalidResource", "resource has no native binding");
    return resources_[id.value];
}
void Dx12PlacedResourceArena::create_views(ResourceId id) {
    auto* object = resource(id);
    const auto& logical = plan_.graph.description.resources[id.value];
    const auto* t = std::get_if<TextureDesc>(&logical.descriptor);
    if (t && t->render_target) device_->CreateRenderTargetView(object, nullptr, rtv_.cpu(id.value));
    if (t && t->depth_stencil) {
        D3D12_DEPTH_STENCIL_VIEW_DESC desc{}; desc.Format = DXGI_FORMAT_D32_FLOAT; desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        device_->CreateDepthStencilView(object, &desc, dsv_.cpu(id.value)); desc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
        device_->CreateDepthStencilView(object, &desc, dsv_.cpu(count_ + id.value));
    }
    bool needs_srv = false;
    for (const auto& pass : plan_.graph.passes) for (const auto& use : pass.usages) if (use.resource == id && use.usage == Usage::ShaderRead) needs_srv = true;
    if (needs_srv) {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc{}; desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        if (t) {
            desc.Format = t->depth_stencil ? DXGI_FORMAT_R32_FLOAT : native_descriptor(logical).Format;
            desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; desc.Texture2D.MipLevels = 1;
        } else {
            const auto bytes = std::get<BufferDesc>(logical.descriptor).bytes;
            if (bytes % 4 || bytes / 4 > UINT32_MAX) throw GpuFailure("BufferView", "raw buffer SRV needs bounded dword size");
            desc.Format = DXGI_FORMAT_R32_TYPELESS; desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER; desc.Buffer.NumElements = static_cast<UINT>(bytes / 4); desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        }
        device_->CreateShaderResourceView(object, &desc, cpu_.cpu(id.value));
        device_->CopyDescriptorsSimple(1, gpu_.cpu(id.value), cpu_.cpu(id.value), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
    const bool uav = t ? t->unordered_access : std::get<BufferDesc>(logical.descriptor).unordered_access;
    if (uav) {
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
        if (t) { desc.Format = native_descriptor(logical).Format; desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D; }
        else {
            const auto bytes = std::get<BufferDesc>(logical.descriptor).bytes;
            if (bytes % 4 || bytes / 4 > UINT32_MAX) throw GpuFailure("BufferView", "raw buffer UAV needs bounded dword size");
            desc.Format = DXGI_FORMAT_R32_TYPELESS; desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER; desc.Buffer.NumElements = static_cast<UINT>(bytes / 4); desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
        }
        device_->CreateUnorderedAccessView(object, nullptr, &desc, cpu_.cpu(count_ + id.value));
        device_->CopyDescriptorsSimple(1, gpu_.cpu(count_ + id.value), cpu_.cpu(count_ + id.value), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
}
D3D12_CPU_DESCRIPTOR_HANDLE Dx12PlacedResourceArena::dsv(ResourceId id, bool read_only) const { return dsv_.cpu(id.value + (read_only ? count_ : 0)); }
D3D12_CPU_DESCRIPTOR_HANDLE Dx12PlacedResourceArena::uav_cpu(ResourceId id) const { return cpu_.cpu(count_ + id.value); }
D3D12_GPU_DESCRIPTOR_HANDLE Dx12PlacedResourceArena::uav_gpu(ResourceId id) const { return gpu_.gpu(count_ + id.value); }
}
