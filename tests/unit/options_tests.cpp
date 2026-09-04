#include "test_support.hpp"
#include "app/options.hpp"

namespace {
std::expected<fgl::Options, fgl::OptionError> parse(std::initializer_list<std::string_view> args) {
    return fgl::parse_options(std::span(args.begin(), args.size()));
}
void error(std::initializer_list<std::string_view> args, fgl::OptionCode expected) {
    const auto value = parse(args); CHECK(!value); CHECK(value.error().code == expected);
}
}
CASE(default_options) {
    const auto o = parse({}); CHECK(o); CHECK(o->width == 1280 && o->height == 720);
    CHECK(o->frames == 0 && !o->headless && o->aliasing); CHECK(o->scene_seed == 24301);
}
CASE(required_cli) {
    const auto o = parse({"--warp", "--headless", "--frames", "240", "--scene-seed", "24301", "--capture", "smoke.png", "--report", "report.json", "--plan", "plan.json", "--aliasing", "off"});
    CHECK(o); CHECK(o->adapter == fgl::AdapterMode::Warp); CHECK(o->frames == 240 && o->headless && !o->aliasing);
    CHECK(o->capture == "smoke.png" && o->report == "report.json");
}
CASE(headless_is_bounded_by_default) { const auto o = parse({"--headless"}); CHECK(o && o->frames == 240); }
CASE(capture_is_bounded_by_default) { const auto o = parse({"--capture", "x.png"}); CHECK(o && o->frames == 240); }
CASE(adapter_conflict_rejected) { error({"--warp", "--hardware"}, fgl::OptionCode::Conflict); }
CASE(unknown_option_rejected) { error({"--worp"}, fgl::OptionCode::Unknown); }
CASE(missing_value_rejected) { error({"--frames"}, fgl::OptionCode::Missing); }
CASE(negative_or_junk_number_rejected) { error({"--frames", "-1"}, fgl::OptionCode::InvalidValue); error({"--frames", "12oops"}, fgl::OptionCode::InvalidValue); }
CASE(unbounded_or_huge_frames_rejected) { error({"--frames", "0"}, fgl::OptionCode::InvalidValue); error({"--frames", "1000001"}, fgl::OptionCode::InvalidValue); }
CASE(invalid_alias_policy_rejected) { error({"--aliasing", "auto"}, fgl::OptionCode::InvalidValue); }
CASE(timeout_and_size_bounds) { error({"--timeout-ms", "0"}, fgl::OptionCode::InvalidValue); error({"--width", "99999"}, fgl::OptionCode::InvalidValue); error({"--height", "0"}, fgl::OptionCode::InvalidValue); }
CASE(max_seed_and_explicit_hardware) {
    const auto o = parse({"--hardware", "--scene-seed", "4294967295", "--frames", "1"});
    CHECK(o && o->scene_seed == UINT32_MAX && o->adapter == fgl::AdapterMode::Hardware);
}
CASE(debug_and_negative_switches) {
    const auto o = parse({"--barrier-trace", "--lifetime-trace", "--validation-undeclared", "--validation-invalid-graph"});
    CHECK(o && o->barrier_trace && o->lifetime_trace && o->validation_undeclared && o->validation_invalid_graph);
}
int main() { return test::run(); }
