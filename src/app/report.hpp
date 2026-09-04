#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace fgl {
struct PassTiming {
    std::string name;
    double cpu_record_ms{}, gpu_ms{};
    std::uint64_t cpu_samples{}, gpu_samples{};
};
struct RuntimeReport {
    bool success{}, debug_enabled{}, dred_enabled{}, software{}, source_clean{};
    std::string backend, adapter, driver, feature_level, git_sha, failure_code, error;
    std::string current_graph{"clear-smoke"}, current_pass{"Clear"}, device_diagnostics;
    std::string plan_identity, pixel_hash;
    std::string draw_mode{"n/a"};
    std::uint64_t logical_bytes{}, committed_bytes{}, planned_heap_bytes{}, actual_heap_bytes{}, all_frame_heap_bytes{}, saved_bytes{};
    std::uint64_t executed_transitions{}, executed_uav_barriers{}, executed_alias_barriers{}, gpu_timestamp_frequency{};
    std::uint32_t placed_resource_count{}, heap_count{}, alias_reuse_events{}, plan_compile_count{};
    std::uint32_t transition_count{}, uav_count{}, aliasing_count{}, color_buckets{};
    std::uint32_t input_instance_count{}, cpu_visible_count{}, gpu_visible_count{};
    double compile_ms{}, non_black_fraction{}, luminance_min{}, luminance_max{};
    std::vector<PassTiming> pass_timings;
    std::uint32_t vendor_id{}, device_id{}, width{}, height{}, frames{}, frames_in_flight{3};
    std::uint64_t debug_errors{}, debug_warnings{}, debug_corruptions{};
    std::uint32_t resize_count{}, minimize_count{}, restore_count{};
    std::int64_t hresult{}, device_removed_reason{};
    std::vector<std::string> debug_messages;
    std::string json() const;
};
}
