#pragma once
#include "platform.hpp"
#include <cstdint>
namespace fgl {
struct InputState { float yaw_delta{}; bool pause{}, step{}, reset{}, alias_toggle{}, debug_next{}; };
class Win32Window {
public:
    Win32Window(std::uint32_t width, std::uint32_t height, bool visible);
    ~Win32Window();
    Win32Window(const Win32Window&) = delete;
    Win32Window& operator=(const Win32Window&) = delete;
    void pump();
    void set_size(std::uint32_t width, std::uint32_t height);
    void minimize_restore();
    void title(const std::wstring& value);
    bool closed() const { return closed_; }
    bool resized() const { return resized_; }
    void acknowledge_resize() { resized_ = false; }
    std::uint32_t width() const { return width_; }
    std::uint32_t height() const { return height_; }
    HWND handle() const { return handle_; }
    InputState take_input();
    std::uint32_t minimize_count{}, restore_count{};
private:
    static LRESULT CALLBACK procedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    HWND handle_{};
    HINSTANCE instance_{};
    ATOM class_{};
    std::uint32_t width_{}, height_{};
    bool visible_{}, closed_{}, resized_{}, minimized_{}, mouse_valid_{};
    int mouse_x_{};
    InputState input_;
};
}
