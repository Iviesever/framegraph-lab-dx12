#include "platform.hpp"
#include <iomanip>
#include <sstream>
namespace fgl {
GpuFailure::GpuFailure(std::string kind, std::string message, HRESULT hr, HRESULT reason, std::string detail)
    : std::runtime_error(std::move(message)), category(std::move(kind)), code(hr), removed_reason(reason), diagnostics(std::move(detail)) {}
void check_hr(HRESULT result, const char* operation, ID3D12Device* device) {
    if (SUCCEEDED(result)) return;
    const auto removed = device ? device->GetDeviceRemovedReason() : S_OK;
    std::ostringstream text; text << operation << " failed: HRESULT=0x" << std::hex << static_cast<unsigned long>(result)
        << " removed_reason=0x" << static_cast<unsigned long>(removed);
    throw GpuFailure(FAILED(removed) ? "DeviceRemoved" : "NativeFailure", text.str(), result, removed,
        FAILED(removed) ? removal_diagnostics(device) : std::string{});
}
std::string removal_diagnostics(ID3D12Device* device) {
    if (!device) return "DRED unavailable: no device";
    ComPtr<ID3D12DeviceRemovedExtendedData1> dred;
    const auto query = device->QueryInterface(IID_PPV_ARGS(&dred));
    if (FAILED(query)) return "DRED interface unavailable HRESULT=" + std::to_string(query);
    std::ostringstream out;
    D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbs{};
    const auto breadcrumb_result = dred->GetAutoBreadcrumbsOutput1(&breadcrumbs);
    out << "breadcrumbs HRESULT=" << breadcrumb_result;
    unsigned count = 0;
    for (auto* node = breadcrumbs.pHeadAutoBreadcrumbNode; node && count < 64; node = node->pNext, ++count) {
        out << " [list=" << (node->pCommandListDebugNameA ? node->pCommandListDebugNameA : "unnamed")
            << " completed=" << (node->pLastBreadcrumbValue ? *node->pLastBreadcrumbValue : 0)
            << '/' << node->BreadcrumbCount << ']';
    }
    D3D12_DRED_PAGE_FAULT_OUTPUT1 fault{};
    const auto fault_result = dred->GetPageFaultAllocationOutput1(&fault);
    out << " page-fault HRESULT=" << fault_result << " address=0x" << std::hex << fault.PageFaultVA;
    return out.str();
}
std::string utf8(const wchar_t* text) {
    const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) throw GpuFailure("Win32", "WideCharToMultiByte size failed", HRESULT_FROM_WIN32(GetLastError()));
    std::string out(static_cast<std::size_t>(size), '\0');
    if (!WideCharToMultiByte(CP_UTF8, 0, text, -1, out.data(), size, nullptr, nullptr))
        throw GpuFailure("Win32", "WideCharToMultiByte failed", HRESULT_FROM_WIN32(GetLastError()));
    out.pop_back(); return out;
}
EventHandle::EventHandle() : value_(CreateEventW(nullptr, FALSE, FALSE, nullptr)) {
    if (!value_) throw GpuFailure("Win32", "CreateEvent failed", HRESULT_FROM_WIN32(GetLastError()));
}
EventHandle::~EventHandle() { if (value_) CloseHandle(value_); }
}
