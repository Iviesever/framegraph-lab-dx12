#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace fgl {
struct RuntimeReport {
    bool success{}, debug_enabled{}, dred_enabled{}, software{}, source_clean{};
    std::string backend, adapter, driver, feature_level, git_sha, failure_code, error;
    std::string current_graph{"clear-smoke"}, current_pass{"Clear"}, device_diagnostics;
    std::uint32_t vendor_id{}, device_id{}, width{}, height{}, frames{}, frames_in_flight{3};
    std::uint64_t debug_errors{}, debug_warnings{}, debug_corruptions{};
    std::uint32_t resize_count{}, minimize_count{}, restore_count{};
    std::int64_t hresult{}, device_removed_reason{};
    std::vector<std::string> debug_messages;
    std::string json() const;
};
}
