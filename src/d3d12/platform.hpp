#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <stdexcept>
#include <string>

namespace fgl {
template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
struct GpuFailure : std::runtime_error {
    std::string category;
    HRESULT code, removed_reason;
    std::string diagnostics;
    GpuFailure(std::string category, std::string message, HRESULT code = E_FAIL, HRESULT reason = S_OK, std::string diagnostics = {});
};
void check_hr(HRESULT result, const char* operation, ID3D12Device* device = nullptr);
std::string utf8(const wchar_t* text);
std::string removal_diagnostics(ID3D12Device* device);
class EventHandle {
public:
    EventHandle();
    ~EventHandle();
    EventHandle(const EventHandle&) = delete;
    EventHandle& operator=(const EventHandle&) = delete;
    HANDLE get() const { return value_; }
private:
    HANDLE value_{};
};
}
