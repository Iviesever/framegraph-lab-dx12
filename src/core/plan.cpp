#include "framegraph/plan.hpp"
#include <locale>
#include <sstream>

namespace framegraph {
Result<CompiledPlan> PlanCompiler::compile(const GraphDescription& description, const std::vector<MemoryRequirement>& requirements, bool aliasing) {
    auto graph = GraphCompiler::compile(description);
    if (!graph) return std::unexpected(graph.error());
    auto allocation = TransientAllocator::plan(*graph, requirements, aliasing);
    if (!allocation) return std::unexpected(allocation.error());
    auto barriers = ResourceStatePlanner::plan(*graph, *allocation);
    if (!barriers) return std::unexpected(barriers.error());
    return CompiledPlan{std::move(*graph), std::move(*allocation), std::move(*barriers)};
}
namespace {
void barriers_json(std::ostream& out, const std::vector<Barrier>& barriers) {
    out << '[';
    for (std::size_t i = 0; i < barriers.size(); ++i) {
        if (i) out << ',';
        const auto& b = barriers[i];
        out << "{\"kind\":" << static_cast<unsigned>(b.kind) << ",\"resource\":" << b.resource.value
            << ",\"before\":" << static_cast<unsigned>(b.before) << ",\"after\":" << static_cast<unsigned>(b.after) << ",\"alias_before\":";
        if (b.alias_before.value == invalid_index) out << "null"; else out << b.alias_before.value;
        out << '}';
    }
    out << ']';
}
std::string payload(const CompiledPlan& p) {
    auto core = canonical_json(p.graph);
    core.resize(core.rfind(",\"plan_identity\":"));
    std::ostringstream out;
    out.imbue(std::locale::classic());
    const auto& a = p.allocation;
    out << core << ",\"allocation\":{\"aliasing_enabled\":" << a.aliasing_enabled
        << ",\"committed_bytes\":" << a.committed_bytes << ",\"physical_bytes\":" << a.physical_bytes
        << ",\"saved_bytes\":" << a.saved_bytes << ",\"padding_overhead_bytes\":" << a.padding_overhead_bytes << ",\"heaps\":[";
    for (std::size_t i = 0; i < a.heaps.size(); ++i) {
        if (i) out << ',';
        const auto& h = a.heaps[i];
        out << "{\"id\":" << h.id << ",\"class\":" << static_cast<unsigned>(h.heap_class) << ",\"compatibility\":" << h.compatibility
            << ",\"bytes\":" << h.bytes << ",\"alignment\":" << h.alignment << ",\"dedicated\":" << h.dedicated << '}';
    }
    out << "],\"resources\":[";
    for (std::size_t i = 0; i < a.resources.size(); ++i) {
        if (i) out << ',';
        if (!a.resources[i]) { out << "null"; continue; }
        const auto& r = *a.resources[i];
        out << "{\"resource\":" << i << ",\"heap\":" << r.heap << ",\"slot\":" << r.slot << ",\"offset\":" << r.offset
            << ",\"size\":" << r.size << ",\"alignment\":" << r.alignment << ",\"reused_region\":" << r.reused_region << ",\"predecessor\":";
        if (r.predecessor.value == invalid_index) out << "null"; else out << r.predecessor.value;
        out << '}';
    }
    out << "],\"aliases\":[";
    for (std::size_t i = 0; i < a.aliases.size(); ++i) {
        if (i) out << ',';
        const auto& e = a.aliases[i];
        out << "{\"position\":" << e.position << ",\"heap\":" << e.heap << ",\"offset\":" << e.offset << ",\"size\":" << e.size
            << ",\"before\":" << e.before.value << ",\"after\":" << e.after.value << '}';
    }
    out << "]},\"barriers\":{\"transition_count\":" << p.barriers.transition_count << ",\"uav_count\":" << p.barriers.uav_count
        << ",\"aliasing_count\":" << p.barriers.aliasing_count << ",\"passes\":[";
    for (std::size_t i = 0; i < p.barriers.passes.size(); ++i) {
        if (i) out << ',';
        out << "{\"before\":"; barriers_json(out, p.barriers.passes[i].before);
        out << ",\"after\":"; barriers_json(out, p.barriers.passes[i].after); out << '}';
    }
    out << "],\"final\":"; barriers_json(out, p.barriers.final); out << "}}";
    return out.str();
}
}
std::string plan_identity(const CompiledPlan& plan) { return stable_hash(payload(plan)); }
std::string canonical_json(const CompiledPlan& plan) {
    auto value = payload(plan); const auto identity = stable_hash(value); value.pop_back();
    return value + ",\"plan_identity\":\"" + identity + "\"}";
}
}
