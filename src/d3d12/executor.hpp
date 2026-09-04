#pragma once
#include "arena.hpp"
#include "capture.hpp"
#include "context.hpp"
#include <array>
#include <functional>
#include <memory>
#include <vector>
namespace fgl {
class Dx12PassContext {
public:
    Dx12PassContext(ID3D12GraphicsCommandList* list, const framegraph::CompiledPass& pass, Dx12PlacedResourceArena& arena, const ReadbackLayout& layout, std::uint32_t logical_frame);
    ID3D12Resource* resource(framegraph::ResourceId id) const;
    D3D12_CPU_DESCRIPTOR_HANDLE rtv(framegraph::ResourceId id) const;
    D3D12_CPU_DESCRIPTOR_HANDLE dsv(framegraph::ResourceId id, bool read_only = false) const;
    D3D12_GPU_DESCRIPTOR_HANDLE srv(framegraph::ResourceId id) const;
    void clear_rtv(framegraph::ResourceId id, const std::array<float, 4>& color);
    void clear_uav(framegraph::ResourceId id, const std::array<std::uint32_t, 4>& values);
    void clear_uav_float(framegraph::ResourceId id, const std::array<float, 4>& values);
    void copy(framegraph::ResourceId source, framegraph::ResourceId destination);
    void readback(framegraph::ResourceId source, framegraph::ResourceId destination);
    void copy_buffer(framegraph::ResourceId source, framegraph::ResourceId destination, std::uint64_t bytes);
    D3D12_GPU_DESCRIPTOR_HANDLE uav(framegraph::ResourceId id) const;
    void indirect(ID3D12CommandSignature* signature, framegraph::ResourceId arguments);
    ID3D12GraphicsCommandList* list() const { return list_; }
    std::uint32_t width() const { return layout_.width; }
    std::uint32_t height() const { return layout_.height; }
    std::uint32_t logical_frame() const { return logical_frame_; }
private:
    const framegraph::ResourceUsage& declared(framegraph::ResourceId id) const;
    void require(framegraph::ResourceId id, framegraph::Usage usage) const;
    ID3D12GraphicsCommandList* list_;
    const framegraph::CompiledPass& pass_;
    Dx12PlacedResourceArena& arena_;
    const ReadbackLayout& layout_;
    std::uint32_t logical_frame_;
};
using PassCallback = std::function<void(Dx12PassContext&)>;
class Dx12GraphExecutor {
public:
    Dx12GraphExecutor(Dx12Context& context, framegraph::GraphDescription graph, std::vector<PassCallback> callbacks,
        framegraph::ResourceId backbuffer, framegraph::ResourceId readback, RuntimeReport& report, bool aliasing);
    Dx12GraphExecutor(Dx12Context& context, framegraph::GraphDescription graph, std::vector<PassCallback> callbacks,
        framegraph::ResourceId backbuffer, framegraph::ResourceId readback, framegraph::ResourceId cull_readback, RuntimeReport& report, bool aliasing);
    ~Dx12GraphExecutor();
    Dx12GraphExecutor(const Dx12GraphExecutor&) = delete;
    Dx12GraphExecutor& operator=(const Dx12GraphExecutor&) = delete;
    void record(std::uint32_t logical_frame);
    void submitted();
    std::vector<std::uint8_t> finish(std::uint32_t capture_timeout_ms);
    const framegraph::CompiledPlan& plan() const { return plan_; }
private:
    struct FrameData;
    void collect(std::uint32_t index);
    void emit(ID3D12GraphicsCommandList* list, Dx12PlacedResourceArena& arena, const std::vector<framegraph::Barrier>& barriers);
    Dx12Context& context_;
    RuntimeReport& report_;
    framegraph::CompiledPlan plan_;
    std::vector<PassCallback> callbacks_;
    framegraph::ResourceId backbuffer_, readback_, cull_readback_;
    ReadbackLayout layout_;
    std::array<std::unique_ptr<FrameData>, frame_count> frames_;
    std::uint64_t frequency_{};
    std::uint32_t last_frame_{};
    bool has_frame_{};
};
}
