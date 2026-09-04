#include "context.hpp"
#include <sstream>
#include <vector>
namespace fgl {
Dx12Context::Dx12Context(const Options& options, RuntimeReport& report) : options_(options), report_(report) {
    ComPtr<ID3D12Debug> debug;
    const auto debug_result = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
    if (FAILED(debug_result)) throw GpuFailure("DebugLayerUnavailable", "D3D12 Debug Layer is required for this validation build", debug_result);
    debug->EnableDebugLayer(); report_.debug_enabled = true;
    ComPtr<ID3D12DeviceRemovedExtendedDataSettings1> dred_settings;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dred_settings)))) {
        dred_settings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
        dred_settings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
        report_.dred_enabled = true;
    }
    check_hr(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory_)), "CreateDXGIFactory2");
    if (options.adapter != AdapterMode::Warp) {
        for (UINT i = 0; i < 128; ++i) {
            ComPtr<IDXGIAdapter1> candidate;
            const auto result = factory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&candidate));
            if (result == DXGI_ERROR_NOT_FOUND) break;
            check_hr(result, "EnumAdapterByGpuPreference");
            if (options.adapter_index != UINT32_MAX && options.adapter_index != i) continue;
            DXGI_ADAPTER_DESC1 description{}; check_hr(candidate->GetDesc1(&description), "GetDesc1");
            if (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
            if (SUCCEEDED(D3D12CreateDevice(candidate.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_)))) { adapter_ = candidate; break; }
        }
    }
    if (!device_) {
        if (options.adapter == AdapterMode::Hardware || options.adapter_index != UINT32_MAX)
            throw GpuFailure("UnsupportedAdapter", "no requested hardware adapter supports D3D12", DXGI_ERROR_UNSUPPORTED);
        check_hr(factory_->EnumWarpAdapter(IID_PPV_ARGS(&adapter_)), "EnumWarpAdapter");
        check_hr(D3D12CreateDevice(adapter_.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_)), "D3D12CreateDevice(WARP)");
    }
    check_hr(device_.As(&info_queue_), "ID3D12InfoQueue");
    DXGI_ADAPTER_DESC1 description{}; check_hr(adapter_->GetDesc1(&description), "GetDesc1");
    report_.adapter = utf8(description.Description); report_.vendor_id = description.VendorId; report_.device_id = description.DeviceId;
    report_.software = (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0; report_.backend = report_.software ? "warp" : "hardware";
    LARGE_INTEGER version{};
    if (SUCCEEDED(adapter_->CheckInterfaceSupport(__uuidof(IDXGIDevice), &version))) {
        report_.driver = std::to_string(HIWORD(version.HighPart)) + '.' + std::to_string(LOWORD(version.HighPart)) + '.'
            + std::to_string(HIWORD(version.LowPart)) + '.' + std::to_string(LOWORD(version.LowPart));
    } else report_.driver = "unavailable via DXGI CheckInterfaceSupport";
    const D3D_FEATURE_LEVEL levels[]{D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    D3D12_FEATURE_DATA_FEATURE_LEVELS supported{static_cast<UINT>(std::size(levels)), levels, D3D_FEATURE_LEVEL_11_0};
    check_hr(device_->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &supported, sizeof(supported)), "CheckFeatureSupport");
    report_.feature_level = std::to_string((supported.MaxSupportedFeatureLevel >> 12) & 15) + '.' + std::to_string((supported.MaxSupportedFeatureLevel >> 8) & 15);
    D3D12_COMMAND_QUEUE_DESC queue_desc{}; queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    check_hr(device_->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&queue_)), "CreateCommandQueue", device_.Get());
    check_hr(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)), "CreateFence", device_.Get());
    for (auto& frame : frames_) {
        check_hr(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame.allocator)), "CreateCommandAllocator", device_.Get());
        check_hr(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame.allocator.Get(), nullptr, IID_PPV_ARGS(&frame.list)), "CreateCommandList", device_.Get());
        check_hr(frame.list->Close(), "initial command list Close", device_.Get());
    }
    window_ = std::make_unique<Win32Window>(options.width, options.height, !options.headless);
    width_ = window_->width(); height_ = window_->height();
    DXGI_SWAP_CHAIN_DESC1 swap{}; swap.Width = width_; swap.Height = height_; swap.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap.SampleDesc.Count = 1; swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; swap.BufferCount = frame_count; swap.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    ComPtr<IDXGISwapChain1> chain;
    check_hr(factory_->CreateSwapChainForHwnd(queue_.Get(), window_->handle(), &swap, nullptr, nullptr, &chain), "CreateSwapChainForHwnd", device_.Get());
    check_hr(chain.As(&swapchain_), "IDXGISwapChain3");
    check_hr(factory_->MakeWindowAssociation(window_->handle(), DXGI_MWA_NO_ALT_ENTER), "MakeWindowAssociation");
    create_backbuffers();
}
Dx12Context::~Dx12Context() {
    try { wait_idle(); } catch (...) { /* Explicit runtime shutdown reports before this noexcept fallback. */ }
}
void Dx12Context::create_backbuffers() {
    D3D12_DESCRIPTOR_HEAP_DESC heap{}; heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; heap.NumDescriptors = frame_count;
    check_hr(device_->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&rtv_heap_)), "CreateDescriptorHeap(RTV)", device_.Get());
    rtv_stride_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    auto handle = rtv_heap_->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < frame_count; ++i) {
        check_hr(swapchain_->GetBuffer(i, IID_PPV_ARGS(&backbuffers_[i])), "swapchain GetBuffer", device_.Get());
        device_->CreateRenderTargetView(backbuffers_[i].Get(), nullptr, handle); handle.ptr += rtv_stride_;
    }
    report_.width = width_; report_.height = height_;
}
void Dx12Context::wait(std::uint64_t value, std::uint32_t timeout_ms) {
    if (!value) return;
    const auto completed = fence_->GetCompletedValue();
    if (completed == UINT64_MAX) throw GpuFailure("DeviceRemoved", "fence reports device removal", DXGI_ERROR_DEVICE_REMOVED, device_->GetDeviceRemovedReason(), removal_diagnostics(device_.Get()));
    if (completed >= value) return;
    check_hr(fence_->SetEventOnCompletion(value, event_.get()), "SetEventOnCompletion", device_.Get());
    const auto result = WaitForSingleObject(event_.get(), timeout_ms ? timeout_ms : options_.timeout_ms);
    if (result == WAIT_TIMEOUT) throw GpuFailure("FenceTimeout", "bounded GPU fence wait timed out at " + current_pass, HRESULT_FROM_WIN32(WAIT_TIMEOUT), device_->GetDeviceRemovedReason());
    if (result != WAIT_OBJECT_0) throw GpuFailure("FenceWait", "GPU event wait failed", HRESULT_FROM_WIN32(GetLastError()));
    check_hr(device_->GetDeviceRemovedReason(), "GetDeviceRemovedReason", device_.Get());
}
void Dx12Context::wait_idle() { if (fence_) for (const auto& frame : frames_) wait(frame.fence_value); }
FrameContext& Dx12Context::begin_frame() {
    current_frame_ = static_cast<std::uint32_t>(frame_number_ % frame_count);
    auto& frame = frames_[current_frame_]; wait(frame.fence_value);
    check_hr(frame.allocator->Reset(), "CommandAllocator Reset", device_.Get());
    check_hr(frame.list->Reset(frame.allocator.Get(), nullptr), "CommandList Reset", device_.Get());
    backbuffer_index_ = swapchain_->GetCurrentBackBufferIndex();
    return frame;
}
void Dx12Context::submit_frame() {
    auto& frame = frames_[current_frame_];
    check_hr(frame.list->Close(), "CommandList Close", device_.Get());
    if (next_fence_ == UINT64_MAX) throw GpuFailure("FenceExhausted", "fence timeline cannot wrap");
    ID3D12CommandList* lists[]{frame.list.Get()}; queue_->ExecuteCommandLists(1, lists);
    const auto present_result = swapchain_->Present(options_.headless ? 0 : 1, 0);
    frame.fence_value = next_fence_++;
    check_hr(queue_->Signal(fence_.Get(), frame.fence_value), "Queue Signal", device_.Get());
    check_hr(present_result, "Present", device_.Get());
    ++frame_number_; report_.frames = static_cast<std::uint32_t>(frame_number_);
}
D3D12_CPU_DESCRIPTOR_HANDLE Dx12Context::backbuffer_rtv() const {
    auto handle = rtv_heap_->GetCPUDescriptorHandleForHeapStart(); handle.ptr += static_cast<SIZE_T>(backbuffer_index_) * rtv_stride_; return handle;
}
bool Dx12Context::sync_size() {
    if (!window_->width() || !window_->height()) return false;
    if (window_->resized()) {
        window_->acknowledge_resize();
        if (width_ != window_->width() || height_ != window_->height()) {
            wait_idle();
            for (auto& buffer : backbuffers_) buffer.Reset();
            rtv_heap_.Reset();
            width_ = window_->width(); height_ = window_->height();
            check_hr(swapchain_->ResizeBuffers(frame_count, width_, height_, DXGI_FORMAT_R8G8B8A8_UNORM, 0), "ResizeBuffers", device_.Get());
            create_backbuffers(); ++report_.resize_count;
        }
    }
    report_.minimize_count = window_->minimize_count; report_.restore_count = window_->restore_count;
    return true;
}
void Dx12Context::collect_debug() {
    const auto count = info_queue_->GetNumStoredMessages();
    if (count > 65536) throw GpuFailure("DebugMessageLimit", "debug message collection exceeds bounded limit");
    for (; message_cursor_ < count; ++message_cursor_) {
        SIZE_T size{}; check_hr(info_queue_->GetMessage(message_cursor_, nullptr, &size), "GetMessage size");
        std::vector<std::byte> bytes(size);
        auto* message = reinterpret_cast<D3D12_MESSAGE*>(bytes.data());
        check_hr(info_queue_->GetMessage(message_cursor_, message, &size), "GetMessage");
        if (message->Severity == D3D12_MESSAGE_SEVERITY_ERROR) ++report_.debug_errors;
        else if (message->Severity == D3D12_MESSAGE_SEVERITY_WARNING) ++report_.debug_warnings;
        else if (message->Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION) ++report_.debug_corruptions;
        else continue;
        report_.debug_messages.push_back("id=" + std::to_string(message->ID) + " " + message->pDescription);
    }
}
}
