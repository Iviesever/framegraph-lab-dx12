#include "report.hpp"
#include "framegraph/graph.hpp"
#include <locale>
#include <sstream>
namespace fgl {
std::string RuntimeReport::json() const {
    using framegraph::json_quote;
    std::ostringstream out; out.imbue(std::locale::classic()); out << std::boolalpha;
    out << "{\"schema_version\":1,\"success\":" << success << ",\"git_sha\":" << json_quote(git_sha)
        << ",\"source_clean\":" << source_clean << ",\"backend\":" << json_quote(backend) << ",\"adapter\":" << json_quote(adapter)
        << ",\"driver\":" << json_quote(driver) << ",\"vendor_id\":" << vendor_id << ",\"device_id\":" << device_id
        << ",\"software\":" << software << ",\"feature_level\":" << json_quote(feature_level)
        << ",\"width\":" << width << ",\"height\":" << height << ",\"frames\":" << frames << ",\"frames_in_flight\":" << frames_in_flight
        << ",\"debug_enabled\":" << debug_enabled << ",\"dred_enabled\":" << dred_enabled << ",\"debug_errors\":" << debug_errors << ",\"debug_warnings\":" << debug_warnings
        << ",\"debug_corruptions\":" << debug_corruptions << ",\"resize_count\":" << resize_count
        << ",\"minimize_count\":" << minimize_count << ",\"restore_count\":" << restore_count << ",\"hresult\":" << hresult
        << ",\"device_removed_reason\":" << device_removed_reason << ",\"failure_code\":" << json_quote(failure_code)
        << ",\"error\":" << json_quote(error) << ",\"current_graph\":" << json_quote(current_graph)
        << ",\"current_pass\":" << json_quote(current_pass) << ",\"device_diagnostics\":" << json_quote(device_diagnostics) << ",\"debug_messages\":[";
    for (std::size_t i = 0; i < debug_messages.size(); ++i) { if (i) out << ','; out << json_quote(debug_messages[i]); }
    return out.str() + "]}";
}
}
