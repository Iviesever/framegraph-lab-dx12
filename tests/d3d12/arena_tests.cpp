#include "d3d12/arena.hpp"
#include "../unit/test_support.hpp"

namespace {
fgl::ComPtr<ID3D12Device> device;

template<class Function>
void expect_descriptor_exhausted(Function&& function) {
    try {
        function();
        CHECK(false);
    } catch (const fgl::GpuFailure& failure) {
        CHECK(failure.category == "DescriptorExhausted");
    }
}
}

CASE(cpu_heap_bounds_are_typed) {
    fgl::DescriptorHeap heap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2);
    CHECK(heap.cpu(0).ptr != 0);
    CHECK(heap.cpu(1).ptr != 0);
    expect_descriptor_exhausted([&] { (void)heap.cpu(2); });
    expect_descriptor_exhausted([&] { (void)heap.gpu(0); });
}

CASE(visible_heap_bounds_are_typed) {
    fgl::DescriptorHeap heap(device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2, true);
    CHECK(heap.cpu(0).ptr != 0);
    CHECK(heap.gpu(0).ptr != 0);
    CHECK(heap.gpu(1).ptr != 0);
    expect_descriptor_exhausted([&] { (void)heap.gpu(2); });
}

int main() {
    fgl::ComPtr<IDXGIFactory4> factory;
    fgl::check_hr(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)), "CreateDXGIFactory2");
    fgl::ComPtr<IDXGIAdapter> warp;
    fgl::check_hr(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp)), "EnumWarpAdapter");
    fgl::check_hr(D3D12CreateDevice(warp.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)), "D3D12CreateDevice(WARP)");
    return test::run();
}
