#pragma once
#include "platform.hpp"
#include "framegraph/plan.hpp"
#include <vector>
namespace fgl {
D3D12_RESOURCE_DESC native_descriptor(const framegraph::ResourceDescription& resource);
std::vector<framegraph::MemoryRequirement> device_requirements(ID3D12Device* device, const framegraph::GraphDescription& graph);
D3D12_RESOURCE_STATES native_state(framegraph::ResourceState state);
class DescriptorHeap {
public:
    DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, std::uint32_t capacity, bool visible = false);
    D3D12_CPU_DESCRIPTOR_HANDLE cpu(std::uint32_t index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE gpu(std::uint32_t index) const;
    ID3D12DescriptorHeap* get() const { return heap_.Get(); }
private:
    ComPtr<ID3D12DescriptorHeap> heap_;
    std::uint32_t capacity_{}, stride_{};
    bool visible_{};
};
class Dx12PlacedResourceArena {
public:
    Dx12PlacedResourceArena(ID3D12Device* device, const framegraph::CompiledPlan& plan);
    void bind_import(framegraph::ResourceId id, ID3D12Resource* resource);
    ID3D12Resource* resource(framegraph::ResourceId id) const;
    D3D12_CPU_DESCRIPTOR_HANDLE rtv(framegraph::ResourceId id) const { return rtv_.cpu(id.value); }
    D3D12_CPU_DESCRIPTOR_HANDLE dsv(framegraph::ResourceId id, bool read_only) const;
    D3D12_GPU_DESCRIPTOR_HANDLE srv(framegraph::ResourceId id) const { return gpu_.gpu(id.value); }
    D3D12_CPU_DESCRIPTOR_HANDLE uav_cpu(framegraph::ResourceId id) const;
    D3D12_GPU_DESCRIPTOR_HANDLE uav_gpu(framegraph::ResourceId id) const;
    ID3D12DescriptorHeap* shader_heap() const { return gpu_.get(); }
    std::uint64_t actual_heap_bytes() const { return heap_bytes_; }
    std::uint32_t placed_count() const { return placed_count_; }
private:
    void create_views(framegraph::ResourceId id);
    ID3D12Device* device_;
    const framegraph::CompiledPlan& plan_;
    std::uint32_t count_;
    std::vector<ComPtr<ID3D12Heap>> heaps_;
    std::vector<ComPtr<ID3D12Resource>> owned_;
    std::vector<ID3D12Resource*> resources_;
    DescriptorHeap rtv_, dsv_, cpu_, gpu_;
    std::uint64_t heap_bytes_{};
    std::uint32_t placed_count_{};
};
}
