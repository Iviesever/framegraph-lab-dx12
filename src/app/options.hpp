#pragma once
#include <cstdint>
#include "framegraph/expected.hpp"
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace fgl {
enum class AdapterMode { Automatic, Hardware, Warp };
enum class SceneMode { NeonRuins, ExecutorProbe };
enum class DrawMode { GpuIndirect, CpuDirect };
struct Options {
    AdapterMode adapter{AdapterMode::Automatic};
    SceneMode scene{SceneMode::NeonRuins};
    DrawMode draw_mode{DrawMode::GpuIndirect};
    bool headless{}, aliasing{true}, resize_stress{}, barrier_trace{}, lifetime_trace{}, validation_undeclared{}, validation_invalid_graph{}, help{};
    std::uint32_t width{1280}, height{720}, frames{}, scene_seed{24301};
    std::uint32_t timeout_ms{10000}, watchdog_ms{300000}, capture_timeout_ms{10000};
    std::uint32_t adapter_index{UINT32_MAX};
    std::filesystem::path capture, report, plan, rgba, shader_directory;
};
enum class OptionCode { Unknown, Missing, InvalidValue, Conflict, Internal };
struct OptionError { OptionCode code; std::string message; };
using OptionResult = framegraph::Expected<Options, OptionError>;
OptionResult parse_options(std::span<const std::string_view> arguments);
std::string usage_text();
}
