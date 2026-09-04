#include "framegraph/plan.hpp"

namespace framegraph {
namespace {
Result<ResourceState> state_for(Usage usage) {
    switch (usage) {
    case Usage::ShaderRead: return ResourceState::ShaderRead;
    case Usage::RenderTarget: return ResourceState::RenderTarget;
    case Usage::DepthWrite: return ResourceState::DepthWrite;
    case Usage::DepthRead: return ResourceState::DepthRead;
    case Usage::UnorderedAccess: return ResourceState::UnorderedAccess;
    case Usage::CopySource: return ResourceState::CopySource;
    case Usage::CopyDest: return ResourceState::CopyDest;
    case Usage::Present: return ResourceState::Present;
    }
    return std::unexpected(GraphError{ErrorCode::InvalidUsage, "unknown state usage", {}, {}, {}});
}
bool equivalent(ResourceState a, ResourceState b) {
    // Legacy D3D12 COMMON and PRESENT both denote the zero state. Core records this
    // explicit semantic equivalence, so backend does not make a barrier decision.
    return a == b || ((a == ResourceState::Common || a == ResourceState::Present)
        && (b == ResourceState::Common || b == ResourceState::Present));
}
}
Result<ResourceStatePlan> ResourceStatePlanner::plan(const CompiledGraph& g, const AllocationPlan& allocation) {
    if (allocation.resources.size() != g.description.resources.size() || g.lifetimes.size() != g.description.resources.size())
        return std::unexpected(GraphError{ErrorCode::InvalidMemoryRequirement, "state planning requires matching allocations and lifetimes", {}, {}, {}});
    ResourceStatePlan result;
    result.passes.resize(g.passes.size());
    std::vector<ResourceState> states;
    std::vector<std::optional<ResourceAccess>> last_access(g.description.resources.size());
    for (const auto& r : g.description.resources) states.push_back(r.initial_state);
    auto transition = [&](std::vector<Barrier>& list, ResourceId resource, ResourceState desired) {
        auto& current = states[resource.value];
        if (!equivalent(current, desired)) { list.push_back({BarrierKind::Transition, resource, current, desired, {}}); ++result.transition_count; }
        current = desired;
    };
    for (std::uint32_t position = 0; position < g.passes.size(); ++position) {
        auto& lists = result.passes[position];
        for (const auto& usage : g.passes[position].usages) {
            const auto id = usage.resource;
            if (id.value >= states.size() || !g.lifetimes[id.value])
                return std::unexpected(GraphError{ErrorCode::InvalidHandle, "compiled usage lacks lifetime", g.passes[position].id, id, {}});
            const auto& physical = allocation.resources[id.value];
            if (!g.description.resources[id.value].imported && !physical)
                return std::unexpected(GraphError{ErrorCode::InvalidMemoryRequirement, "active transient lacks allocation", g.passes[position].id, id, {}});
            // Use the simple placed-resource activation model even for nonaliased
            // resources. Reference mode has activation barriers but no reuse events.
            if (physical && g.lifetimes[id.value]->first == position) {
                lists.before.push_back({BarrierKind::Aliasing, id, ResourceState::Common, ResourceState::Common, physical->predecessor, true});
                ++result.aliasing_count;
            }
            auto desired = state_for(usage.usage);
            if (!desired) return std::unexpected(desired.error());
            if (*desired == ResourceState::UnorderedAccess && states[id.value] == *desired && last_access[id.value]
                && (*last_access[id.value] != ResourceAccess::Read || usage.access != ResourceAccess::Read)) {
                lists.before.push_back({BarrierKind::Uav, id, *desired, *desired, {}}); ++result.uav_count;
            }
            transition(lists.before, id, *desired);
            last_access[id.value] = usage.access;
        }
        for (std::uint32_t i = 0; i < g.lifetimes.size(); ++i) {
            if (!g.description.resources[i].imported && g.lifetimes[i] && g.lifetimes[i]->last == position)
                transition(lists.after, ResourceId{i}, ResourceState::Common);
        }
    }
    for (std::uint32_t i = 0; i < g.description.resources.size(); ++i) {
        const auto& r = g.description.resources[i];
        if (r.imported && r.final_state) transition(result.final, ResourceId{i}, *r.final_state);
    }
    return result;
}
}
