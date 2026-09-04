#include "options.hpp"
#include <charconv>
namespace fgl {
std::expected<Options, OptionError> parse_options(std::span<const std::string_view> arguments) {
    Options options;
    bool adapter_set = false, frames_set = false;
    auto fail = [](OptionCode code, std::string text) -> std::expected<Options, OptionError> {
        return std::unexpected(OptionError{code, std::move(text)});
    };
    for (std::size_t i = 0; i < arguments.size(); ++i) {
        const auto key = arguments[i];
        if (key == "--hardware" || key == "--warp") {
            const auto mode = key == "--warp" ? AdapterMode::Warp : AdapterMode::Hardware;
            if (adapter_set && options.adapter != mode) return fail(OptionCode::Conflict, "--hardware and --warp are mutually exclusive");
            adapter_set = true; options.adapter = mode; continue;
        }
        if (key == "--headless") { options.headless = true; continue; }
        if (key == "--resize-stress") { options.resize_stress = true; continue; }
        if (key == "--barrier-trace") { options.barrier_trace = true; continue; }
        if (key == "--lifetime-trace") { options.lifetime_trace = true; continue; }
        if (key == "--validation-undeclared") { options.validation_undeclared = true; continue; }
        if (key == "--validation-invalid-graph") { options.validation_invalid_graph = true; continue; }
        if (key == "--help" || key == "-h") { options.help = true; continue; }
        const bool numeric = key == "--frames" || key == "--scene-seed" || key == "--width" || key == "--height"
            || key == "--timeout-ms" || key == "--watchdog-ms" || key == "--capture-timeout-ms" || key == "--adapter-index";
        const bool path = key == "--capture" || key == "--report" || key == "--plan" || key == "--rgba" || key == "--shader-dir";
        if (!numeric && !path && key != "--aliasing") return fail(OptionCode::Unknown, "unknown option: " + std::string(key));
        if (++i == arguments.size()) return fail(OptionCode::Missing, "missing value for " + std::string(key));
        const auto value = arguments[i];
        if (path) {
            if (value.empty() || value.starts_with("--")) return fail(OptionCode::InvalidValue, "invalid output/shader path");
            std::u8string encoded(value.size(), u8'\0');
            for (std::size_t byte = 0; byte < value.size(); ++byte) encoded[byte] = static_cast<char8_t>(static_cast<unsigned char>(value[byte]));
            auto p = std::filesystem::path(encoded);
            if (key == "--capture") options.capture = p;
            if (key == "--report") options.report = p;
            if (key == "--plan") options.plan = p;
            if (key == "--rgba") options.rgba = p;
            if (key == "--shader-dir") options.shader_directory = p;
        } else if (key == "--aliasing") {
            if (value != "on" && value != "off") return fail(OptionCode::InvalidValue, "aliasing must be on or off");
            options.aliasing = value == "on";
        } else {
            std::uint32_t n{};
            const auto [end, code] = std::from_chars(value.data(), value.data() + value.size(), n);
            if (code != std::errc{} || end != value.data() + value.size()) return fail(OptionCode::InvalidValue, "expected bounded unsigned integer for " + std::string(key));
            if (key == "--frames") {
                if (!n || n > 1000000) return fail(OptionCode::InvalidValue, "frames must be in [1,1000000]");
                options.frames = n; frames_set = true;
            } else if (key == "--width" || key == "--height") {
                if (n < 64 || n > 8192) return fail(OptionCode::InvalidValue, "dimensions must be in [64,8192]");
                (key == "--width" ? options.width : options.height) = n;
            } else if (key == "--scene-seed") options.scene_seed = n;
            else if (key == "--adapter-index") options.adapter_index = n;
            else {
                if ((!n && key != "--capture-timeout-ms") || n > 3600000) return fail(OptionCode::InvalidValue, "invalid timeout bound");
                if (key == "--timeout-ms") options.timeout_ms = n;
                if (key == "--watchdog-ms") options.watchdog_ms = n;
                if (key == "--capture-timeout-ms") options.capture_timeout_ms = n;
            }
        }
    }
    if (!frames_set && (options.headless || !options.capture.empty() || !options.report.empty() || options.resize_stress)) options.frames = 240;
    return options;
}
std::string usage_text() {
    return "FrameGraphLab 0.1 candidate\n"
        "  --hardware | --warp   --headless   --frames N   --scene-seed N\n"
        "  --capture image.png   --report report.json   --plan plan.json\n"
        "  --aliasing on|off   --rgba pixels.rgba   --width N --height N\n"
        "  --timeout-ms N   --watchdog-ms N   --capture-timeout-ms N\n"
        "  --resize-stress   --adapter-index N   --shader-dir path\n"
        "  --barrier-trace   --lifetime-trace\n";
}
}
