#pragma once
#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace fgl {
enum class AdapterMode { Automatic, Hardware, Warp };
struct Options {
    AdapterMode adapter{AdapterMode::Automatic};
    bool headless{}, aliasing{true}, resize_stress{}, help{};
    std::uint32_t width{1280}, height{720}, frames{}, scene_seed{24301};
    std::uint32_t timeout_ms{10000}, watchdog_ms{300000}, capture_timeout_ms{10000};
    std::uint32_t adapter_index{UINT32_MAX};
    std::filesystem::path capture, report, plan, rgba, shader_directory;
};
enum class OptionCode { Unknown, Missing, InvalidValue, Conflict, Internal };
struct OptionError { OptionCode code; std::string message; };
std::expected<Options, OptionError> parse_options(std::span<const std::string_view> arguments);
std::string usage_text();
}
