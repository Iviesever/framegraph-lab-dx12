#pragma once
#include "../unit/test_support.hpp"
#include "framegraph/plan.hpp"
#include <algorithm>
#include <map>

namespace property {
inline void verify_plan(const framegraph::CompiledPlan& p, const std::vector<framegraph::MemoryRequirement>& req) {
    using namespace framegraph;
    const auto& g = p.graph;
    const auto& a = p.allocation;
    CHECK(a.resources.size() == g.description.resources.size());
    CHECK(p.barriers.passes.size() == g.passes.size());
    std::uint64_t actual_total = 0, reference_total = 0;
    for (const auto& heap : a.heaps) {
        CHECK(heap.id < a.heaps.size()); CHECK(heap.bytes % heap.alignment == 0); actual_total += heap.bytes;
    }
    CHECK(actual_total == a.physical_bytes);
    std::map<std::pair<std::uint32_t, std::uint32_t>, std::vector<std::uint32_t>> chains;
    for (std::uint32_t r = 0; r < a.resources.size(); ++r) {
        const bool allocated = !g.description.resources[r].imported && g.lifetimes[r].has_value();
        CHECK(a.resources[r].has_value() == allocated);
        if (!allocated) continue;
        const auto& v = *a.resources[r];
        CHECK(v.heap < a.heaps.size());
        const auto& heap = a.heaps[v.heap];
        CHECK(v.alignment == req[r].alignment); CHECK(v.size == req[r].size);
        CHECK(v.offset % req[r].alignment == 0); CHECK(v.offset <= heap.bytes); CHECK(v.size <= heap.bytes - v.offset);
        CHECK(heap.heap_class == req[r].heap_class); CHECK(heap.compatibility == req[r].compatibility);
        CHECK(heap.dedicated == req[r].dedicated);
        reference_total += ((req[r].size - 1) / req[r].alignment + 1) * req[r].alignment;
        chains[{v.heap, v.slot}].push_back(r);
    }
    CHECK(reference_total == a.committed_bytes);
    CHECK(a.committed_bytes + a.padding_overhead_bytes == a.physical_bytes + a.saved_bytes);
    // Check actual byte intersections at EVERY active schedule position, independently
    // of slot identity and without calling the allocator's overlap predicate.
    for (std::uint32_t position = 0; position < g.passes.size(); ++position) {
        for (std::uint32_t x = 0; x < a.resources.size(); ++x) {
            if (!a.resources[x] || g.lifetimes[x]->first > position || g.lifetimes[x]->last < position) continue;
            for (auto y = x + 1; y < a.resources.size(); ++y) {
                if (!a.resources[y] || g.lifetimes[y]->first > position || g.lifetimes[y]->last < position) continue;
                const auto& v = *a.resources[x]; const auto& w = *a.resources[y];
                if (v.heap == w.heap) CHECK(v.offset + v.size <= w.offset || w.offset + w.size <= v.offset);
            }
        }
    }
    unsigned aliases = 0, activations = 0;
    for (auto& [key, ids] : chains) {
        (void)key;
        std::sort(ids.begin(), ids.end(), [&](auto x, auto y) { return g.lifetimes[x]->first < g.lifetimes[y]->first; });
        CHECK(a.aliasing_enabled || ids.size() == 1);
        for (std::size_t k = 0; k < ids.size(); ++k) {
            const auto r = ids[k]; const auto& v = *a.resources[r];
            CHECK(v.reused_region == (ids.size() > 1));
            CHECK(v.predecessor == (k ? ResourceId{ids[k - 1]} : ResourceId{}));
            if (req[r].dedicated) CHECK(ids.size() == 1);
            {
                ++activations;
                const auto& barriers = p.barriers.passes[g.lifetimes[r]->first].before;
                unsigned matching = 0;
                for (const auto& b : barriers) if (b.kind == BarrierKind::Aliasing && b.resource == ResourceId{r} && b.alias_before == v.predecessor) ++matching;
                CHECK(matching == 1);
            }
            if (k) {
                ++aliases; CHECK(g.lifetimes[ids[k - 1]]->last < g.lifetimes[r]->first);
                CHECK(a.resources[ids[k - 1]]->offset == v.offset);
                unsigned matching = 0;
                for (const auto& event : a.aliases)
                    if (event.before == v.predecessor && event.after == ResourceId{r} && event.position == g.lifetimes[r]->first
                        && event.heap == v.heap && event.offset == v.offset) ++matching;
                CHECK(matching == 1);
            }
        }
    }
    CHECK(aliases == a.aliases.size()); CHECK(activations == p.barriers.aliasing_count);
    std::vector<ResourceState> state;
    std::vector<bool> active(a.resources.size(), true);
    for (std::size_t r = 0; r < a.resources.size(); ++r)
        if (a.resources[r]) active[r] = false;
    std::vector<std::optional<ResourceAccess>> previous_access(a.resources.size());
    for (const auto& resource : g.description.resources) state.push_back(resource.initial_state);
    auto native_state = [](ResourceState s) { return s == ResourceState::Present ? ResourceState::Common : s; };
    unsigned transition_count = 0, uav_count = 0, alias_count = 0;
    auto apply = [&](const std::vector<Barrier>& barriers) {
        for (const auto& b : barriers) {
            CHECK(b.resource.value < state.size());
            if (b.kind == BarrierKind::Transition) {
                CHECK(active[b.resource.value]);
                ++transition_count; CHECK(native_state(state[b.resource.value]) == native_state(b.before));
                CHECK(native_state(b.before) != native_state(b.after)); state[b.resource.value] = b.after;
            } else if (b.kind == BarrierKind::Uav) {
                CHECK(active[b.resource.value]);
                ++uav_count; CHECK(state[b.resource.value] == ResourceState::UnorderedAccess);
            } else {
                ++alias_count;
                CHECK(a.resources[b.resource.value]);
                if (b.alias_before.value != invalid_index) {
                    CHECK(active[b.alias_before.value]); CHECK(state[b.alias_before.value] == ResourceState::Common);
                }
                const auto& successor = *a.resources[b.resource.value];
                for (std::size_t r = 0; r < a.resources.size(); ++r) {
                    if (!a.resources[r]) continue;
                    const auto& prior = *a.resources[r];
                    if (prior.heap == successor.heap && prior.offset < successor.offset + successor.size && successor.offset < prior.offset + prior.size)
                        active[r] = false;
                }
                active[b.resource.value] = true;
            }
        }
    };
    for (std::uint32_t i = 0; i < g.passes.size(); ++i) {
        const auto before = state;
        apply(p.barriers.passes[i].before);
        for (const auto& u : g.passes[i].usages) {
            CHECK(active[u.resource.value]);
            // State/Usage enumerations intentionally have a one-position offset.
            const auto expected = static_cast<ResourceState>(static_cast<unsigned>(u.usage) + 1);
            CHECK(native_state(state[u.resource.value]) == native_state(expected));
            unsigned uav_barriers = 0;
            for (const auto& b : p.barriers.passes[i].before) if (b.kind == BarrierKind::Uav && b.resource == u.resource) ++uav_barriers;
            const bool ordering_required = expected == ResourceState::UnorderedAccess && before[u.resource.value] == expected
                && previous_access[u.resource.value] && (*previous_access[u.resource.value] != ResourceAccess::Read || u.access != ResourceAccess::Read);
            CHECK(uav_barriers == static_cast<unsigned>(ordering_required));
            previous_access[u.resource.value] = u.access;
        }
        apply(p.barriers.passes[i].after);
    }
    apply(p.barriers.final);
    for (std::uint32_t r = 0; r < state.size(); ++r) {
        const auto& desc = g.description.resources[r];
        if (a.resources[r]) CHECK(state[r] == ResourceState::Common);
        if (desc.imported && desc.final_state) CHECK(native_state(state[r]) == native_state(*desc.final_state));
    }
    CHECK(transition_count == p.barriers.transition_count); CHECK(uav_count == p.barriers.uav_count); CHECK(alias_count == p.barriers.aliasing_count);
}
}
