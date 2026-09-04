#include "framegraph/graph.hpp"
#include "validation.hpp"
#include <algorithm>
#include <functional>
#include <queue>

namespace framegraph {
namespace {
std::vector<PassId> find_cycle(const std::vector<std::vector<std::uint32_t>>& next) {
    struct Frame { std::uint32_t node; std::size_t child; };
    std::vector<unsigned char> color(next.size());
    std::vector<Frame> stack;
    for (std::uint32_t start = 0; start < next.size(); ++start) {
        if (color[start]) continue;
        stack.push_back({start, 0}); color[start] = 1;
        while (!stack.empty()) {
            auto& frame = stack.back();
            if (frame.child == next[frame.node].size()) {
                color[frame.node] = 2; stack.pop_back(); continue;
            }
            const auto child = next[frame.node][frame.child++];
            if (color[child] == 1) {
                std::vector<PassId> chain;
                const auto begin = std::find_if(stack.begin(), stack.end(), [child](const Frame& f) { return f.node == child; });
                for (auto it = begin; it != stack.end(); ++it) chain.push_back(PassId{it->node});
                chain.push_back(PassId{child});
                return chain;
            }
            if (!color[child]) { color[child] = 1; stack.push_back({child, 0}); }
        }
    }
    return {};
}
}
Result<CompiledGraph> GraphCompiler::compile(const GraphDescription& description) {
    if (auto result = detail::validate(description); !result) return unexpected(result.error());
    CompiledGraph graph;
    graph.description = description;
    auto& source = graph.description;
    for (auto& pass : source.passes) std::sort(pass.usages.begin(), pass.usages.end());
    std::sort(source.ordering.begin(), source.ordering.end());
    source.ordering.erase(std::unique(source.ordering.begin(), source.ordering.end()), source.ordering.end());

    std::vector<PassId> writer(source.resources.size());
    std::vector<std::vector<PassId>> readers(source.resources.size());
    std::vector<DependencyEdge> edges;
    auto edge = [&](PassId a, PassId b, ResourceId r, Hazard reason) {
        if (a.value != invalid_index && edges.size() <= max_edges) edges.push_back({a, b, r, reason});
    };
    for (std::uint32_t i = 0; i < source.passes.size(); ++i) {
        const PassId id{i};
        for (const auto& u : source.passes[i].usages) {
            const auto r = u.resource.value;
            if (detail::reads(u.access)) edge(writer[r], id, u.resource, Hazard::Raw);
            if (detail::writes(u.access)) {
                edge(writer[r], id, u.resource, Hazard::Waw);
                for (const auto reader : readers[r]) edge(reader, id, u.resource, Hazard::War);
                readers[r].clear(); writer[r] = id;
            } else readers[r].push_back(id);
        }
    }
    for (const auto& e : source.ordering) edge(e.before, e.after, {}, Hazard::Explicit);
    if (edges.size() > max_edges)
        return unexpected(GraphError{ErrorCode::LimitExceeded, "derived dependencies exceed edge limit", {}, {}, {}});
    std::sort(edges.begin(), edges.end());
    edges.erase(std::unique(edges.begin(), edges.end()), edges.end());

    std::vector<std::vector<std::uint32_t>> next(source.passes.size()), previous(source.passes.size());
    std::vector<std::uint32_t> indegree(source.passes.size());
    OrderingEdge last;
    for (const auto& e : edges) {
        const OrderingEdge pair{e.before, e.after};
        if (pair == last) continue;
        last = pair;
        next[e.before.value].push_back(e.after.value);
        previous[e.after.value].push_back(e.before.value);
        ++indegree[e.after.value];
    }
    std::priority_queue<std::uint32_t, std::vector<std::uint32_t>, std::greater<>> ready;
    for (std::uint32_t i = 0; i < indegree.size(); ++i) if (!indegree[i]) ready.push(i);
    std::vector<std::uint32_t> order;
    while (!ready.empty()) {
        const auto id = ready.top(); ready.pop(); order.push_back(id);
        for (const auto child : next[id]) if (--indegree[child] == 0) ready.push(child);
    }
    if (order.size() != source.passes.size()) {
        auto cycle = find_cycle(next);
        std::string diagnostic = "cycle";
        for (const auto p : cycle) diagnostic += " -> " + source.passes[p.value].name + "[" + std::to_string(p.value) + "]";
        return unexpected(GraphError{ErrorCode::Cycle, std::move(diagnostic), {}, {}, std::move(cycle)});
    }
    std::vector<bool> alive(source.passes.size());
    std::vector<std::uint32_t> pending;
    auto retain = [&](std::uint32_t p) {
        if (p != invalid_index && !alive[p]) { alive[p] = true; pending.push_back(p); }
    };
    for (std::uint32_t i = 0; i < source.passes.size(); ++i) {
        if (source.passes[i].side_effect) retain(i);
        for (const auto& u : source.passes[i].usages) if (u.usage == Usage::Present) retain(i);
    }
    for (std::uint32_t i = 0; i < source.resources.size(); ++i) if (source.resources[i].exported) retain(writer[i].value);
    while (!pending.empty()) {
        const auto p = pending.back(); pending.pop_back();
        for (const auto before : previous[p]) retain(before);
    }
    graph.lifetimes.resize(source.resources.size());
    for (const auto id : order) {
        if (!alive[id]) continue;
        const auto position = static_cast<std::uint32_t>(graph.passes.size());
        graph.passes.push_back({PassId{id}, source.passes[id].usages});
        for (const auto& u : source.passes[id].usages) {
            auto& life = graph.lifetimes[u.resource.value];
            if (!life) life = ResourceLifetime{position, position};
            else life->last = position;
        }
    }
    // Export promises readable contents at graph completion, beyond last declared use.
    for (std::size_t i = 0; i < source.resources.size(); ++i)
        if (source.resources[i].exported && graph.lifetimes[i])
            graph.lifetimes[i]->last = static_cast<std::uint32_t>(graph.passes.size() - 1);
    for (std::uint32_t i = 0; i < alive.size(); ++i) if (!alive[i]) graph.culled.push_back(PassId{i});
    for (const auto& e : edges) if (alive[e.before.value] && alive[e.after.value]) graph.dependencies.push_back(e);
    return graph;
}
Result<ResourceId> GraphBuilder::add_resource(ResourceDescription resource) {
    if (graph_.resources.size() >= max_resources)
        return unexpected(GraphError{ErrorCode::LimitExceeded, "resource limit", {}, {}, {}});
    ResourceId id{static_cast<std::uint32_t>(graph_.resources.size())};
    graph_.resources.push_back(std::move(resource));
    return id;
}
Result<PassId> GraphBuilder::add_pass(PassDescription pass) {
    if (graph_.passes.size() >= max_passes)
        return unexpected(GraphError{ErrorCode::LimitExceeded, "pass limit", {}, {}, {}});
    PassId id{static_cast<std::uint32_t>(graph_.passes.size())};
    graph_.passes.push_back(std::move(pass));
    return id;
}
Result<void> GraphBuilder::order(PassId before, PassId after) {
    if (before.value >= graph_.passes.size() || after.value >= graph_.passes.size())
        return unexpected(GraphError{ErrorCode::InvalidHandle, "ordering references unknown pass", {}, {}, {}});
    if (graph_.ordering.size() >= max_edges)
        return unexpected(GraphError{ErrorCode::LimitExceeded, "ordering limit", {}, {}, {}});
    graph_.ordering.push_back({before, after});
    return {};
}
} // namespace framegraph
