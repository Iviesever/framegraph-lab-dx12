#include "window.hpp"
namespace fgl {
namespace { constexpr wchar_t class_name[] = L"FrameGraphLabNativeWindow"; }
Win32Window::Win32Window(std::uint32_t width, std::uint32_t height, bool visible)
    : instance_(GetModuleHandleW(nullptr)), width_(width), height_(height), visible_(visible) {
    WNDCLASSEXW definition{}; definition.cbSize = sizeof(definition); definition.lpfnWndProc = procedure;
    definition.hInstance = instance_; definition.lpszClassName = class_name;
    definition.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW, explicitly wide.
    class_ = RegisterClassExW(&definition);
    if (!class_) throw GpuFailure("Win32", "RegisterClassEx failed", HRESULT_FROM_WIN32(GetLastError()));
    RECT rect{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
    handle_ = CreateWindowExW(0, class_name, L"FrameGraphLab - runtime smoke", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, instance_, this);
    if (!handle_) { UnregisterClassW(class_name, instance_); class_ = 0; throw GpuFailure("Win32", "CreateWindowEx failed", HRESULT_FROM_WIN32(GetLastError())); }
    if (visible_) ShowWindow(handle_, SW_SHOW);
    resized_ = false;
}
Win32Window::~Win32Window() {
    if (handle_) DestroyWindow(handle_);
    if (class_) UnregisterClassW(class_name, instance_);
}
void Win32Window::pump() {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) closed_ = true;
        TranslateMessage(&message); DispatchMessageW(&message);
    }
}
void Win32Window::set_size(std::uint32_t width, std::uint32_t height) {
    RECT r{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, FALSE, 0);
    if (!SetWindowPos(handle_, nullptr, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE))
        throw GpuFailure("Win32", "SetWindowPos failed", HRESULT_FROM_WIN32(GetLastError()));
    pump();
}
void Win32Window::minimize_restore() {
    ShowWindow(handle_, SW_MINIMIZE); pump();
    ShowWindow(handle_, SW_RESTORE); pump();
    if (!visible_) ShowWindow(handle_, SW_HIDE);
}
void Win32Window::title(const std::wstring& value) { SetWindowTextW(handle_, value.c_str()); }
InputState Win32Window::take_input() { const auto result = input_; input_ = {}; return result; }
LRESULT CALLBACK Win32Window::procedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    auto* self = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        self = static_cast<Win32Window*>(reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams);
        self->handle_ = hwnd; SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    if (self) {
        switch (message) {
        case WM_SIZE:
            self->width_ = LOWORD(lparam); self->height_ = HIWORD(lparam); self->resized_ = true;
            if (wparam == SIZE_MINIMIZED && !self->minimized_) { self->minimized_ = true; ++self->minimize_count; }
            else if (wparam != SIZE_MINIMIZED && self->minimized_) { self->minimized_ = false; ++self->restore_count; }
            return 0;
        case WM_KEYDOWN:
            if (wparam == VK_ESCAPE) { DestroyWindow(hwnd); return 0; }
            if (wparam == VK_LEFT) self->input_.yaw_delta -= 0.06f;
            if (wparam == VK_RIGHT) self->input_.yaw_delta += 0.06f;
            if (!(lparam & (1ll << 30))) {
                if (wparam == VK_SPACE) self->input_.pause = true;
                if (wparam == 'N') self->input_.step = true;
                if (wparam == 'R') self->input_.reset = true;
                if (wparam == 'A') self->input_.alias_toggle = true;
                if (wparam == 'V') self->input_.debug_next = true;
            }
            return 0;
        case WM_MOUSEMOVE: {
            const int x = static_cast<short>(LOWORD(lparam));
            if ((wparam & MK_LBUTTON) && self->mouse_valid_) self->input_.yaw_delta += static_cast<float>(x - self->mouse_x_) * 0.006f;
            self->mouse_x_ = x; self->mouse_valid_ = true; return 0;
        }
        case WM_CLOSE: DestroyWindow(hwnd); return 0;
        case WM_DESTROY: self->closed_ = true; PostQuitMessage(0); return 0;
        case WM_NCDESTROY: self->handle_ = nullptr; SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0); break;
        }
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}
}
