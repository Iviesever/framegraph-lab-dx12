#pragma once
#include "platform.hpp"
#include <d3d12sdklayers.h>
#include "window.hpp"
#include "app/options.hpp"
#include "app/report.hpp"
#include <array>
#include <memory>
namespace fgl {
inline constexpr std::uint32_t frame_count = 3;
struct FrameContext {
    ComPtr<ID3D12CommandAllocator> allocator;
    ComPtr<ID3D12GraphicsCommandList> list;
    std::uint64_t fence_value{};
};
class Dx12Context {
public:
    Dx12Context(const Options& options, RuntimeReport& report);
    ~Dx12Context();
    Dx12Context(const Dx12Context&) = delete;
    Dx12Context& operator=(const Dx12Context&) = delete;
    FrameContext& begin_frame();
    void submit_frame();
    void wait_idle();
    void wait(std::uint64_t value);
    bool sync_size();
    void collect_debug();
    ID3D12Device* device() const { return device_.Get(); }
    ID3D12CommandQueue* queue() const { return queue_.Get(); }
    ID3D12Resource* backbuffer() const { return backbuffers_[backbuffer_index_].Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE backbuffer_rtv() const;
    Win32Window& window() { return *window_; }
    std::uint32_t frame_index() const { return current_frame_; }
    std::uint32_t width() const { return width_; }
    std::uint32_t height() const { return height_; }
    std::string current_pass{"Clear"};
private:
    void create_backbuffers();
    Options options_;
    RuntimeReport& report_;
    std::unique_ptr<Win32Window> window_;
    ComPtr<IDXGIFactory6> factory_;
    ComPtr<IDXGIAdapter1> adapter_;
    ComPtr<ID3D12Device> device_;
    ComPtr<ID3D12InfoQueue> info_queue_;
    ComPtr<ID3D12CommandQueue> queue_;
    ComPtr<IDXGISwapChain3> swapchain_;
    ComPtr<ID3D12DescriptorHeap> rtv_heap_;
    std::array<ComPtr<ID3D12Resource>, frame_count> backbuffers_;
    ComPtr<ID3D12Fence> fence_;
    EventHandle event_;
    std::array<FrameContext, frame_count> frames_;
    std::uint32_t width_{}, height_{}, current_frame_{}, backbuffer_index_{}, rtv_stride_{};
    std::uint64_t frame_number_{}, next_fence_{1}, message_cursor_{};
};
}
