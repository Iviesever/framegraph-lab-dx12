#include "framegraph/plan.hpp"
#include <algorithm>
#include <bit>
#include <tuple>

namespace framegraph {
namespace {
bool add(std::uint64_t a, std::uint64_t b, std::uint64_t& out) {
    if (b > UINT64_MAX - a) return false;
    out = a + b; return true;
}
bool align(std::uint64_t value, std::uint64_t alignment, std::uint64_t& out) {
    std::uint64_t sum{};
    if (!add(value, alignment - 1, sum)) return false;
    out = sum & ~(alignment - 1); return true;
}
}
Result<AllocationPlan> TransientAllocator::plan(const CompiledGraph& g, const std::vector<MemoryRequirement>& requirements, bool aliasing) {
    auto failure = [](ErrorCode code, const char* text, ResourceId r = {}) -> Result<AllocationPlan> {
        return unexpected(GraphError{code, text, {}, r, {}});
    };
    if (requirements.size() != g.description.resources.size() || g.lifetimes.size() != g.description.resources.size())
        return failure(ErrorCode::InvalidMemoryRequirement, "requirements/lifetimes must be indexed by every resource");
    AllocationPlan plan;
    plan.aliasing_enabled = aliasing;
    plan.resources.resize(requirements.size());
    std::vector<ResourceId> order;
    for (std::uint32_t i = 0; i < requirements.size(); ++i) {
        if (!g.lifetimes[i] || g.description.resources[i].imported) continue;
        const auto& life = *g.lifetimes[i];
        if (life.first > life.last || life.last >= g.passes.size())
            return failure(ErrorCode::InvalidMemoryRequirement, "lifetime outside execution schedule", ResourceId{i});
        const auto& req = requirements[i];
        const auto& desc = g.description.resources[i].descriptor;
        const auto* texture = std::get_if<TextureDesc>(&desc);
        const auto expected_class = !texture ? HeapClass::Buffer : texture->render_target || texture->depth_stencil ? HeapClass::RtDsTexture : HeapClass::Texture;
        if (!req.size || !std::has_single_bit(req.alignment) || req.heap_class != expected_class)
            return failure(ErrorCode::InvalidMemoryRequirement, "nonzero size, power-of-two alignment and matching heap class required", ResourceId{i});
        std::uint64_t committed{};
        if (!align(req.size, req.alignment, committed) || !add(plan.committed_bytes, committed, plan.committed_bytes))
            return failure(ErrorCode::Overflow, "committed byte arithmetic overflow", ResourceId{i});
        order.push_back(ResourceId{i});
    }
    std::sort(order.begin(), order.end(), [&](ResourceId a, ResourceId b) {
        return std::tie(g.lifetimes[a.value]->first, a.value) < std::tie(g.lifetimes[b.value]->first, b.value);
    });
    struct Slot {
        std::uint32_t heap;
        std::uint64_t offset, capacity;
        ResourceId owner;
        std::uint32_t last, activations;
    };
    std::vector<Slot> slots;
    for (const auto resource : order) {
        const auto& req = requirements[resource.value];
        const auto& life = *g.lifetimes[resource.value];
        std::uint32_t selected = invalid_index;
        if (aliasing && !req.dedicated) {
            for (std::uint32_t i = 0; i < slots.size(); ++i) {
                const auto& slot = slots[i];
                const auto& heap = plan.heaps[slot.heap];
                if (heap.dedicated || heap.heap_class != req.heap_class || heap.compatibility != req.compatibility
                    || slot.last >= life.first || slot.capacity < req.size || slot.offset % req.alignment) continue;
                if (selected == invalid_index || slot.capacity < slots[selected].capacity) selected = i;
            }
        }
        ResourceId predecessor;
        if (selected != invalid_index) {
            auto& slot = slots[selected];
            predecessor = slot.owner;
            plan.aliases.push_back({life.first, slot.heap, slot.offset,
                std::min(requirements[predecessor.value].size, req.size), predecessor, resource});
            slot.owner = resource; slot.last = life.last; ++slot.activations;
            plan.heaps[slot.heap].alignment = std::max(plan.heaps[slot.heap].alignment, req.alignment);
        } else {
            std::uint32_t heap_id = invalid_index;
            if (!req.dedicated) {
                for (const auto& heap : plan.heaps) {
                    if (!heap.dedicated && heap.heap_class == req.heap_class && heap.compatibility == req.compatibility) { heap_id = heap.id; break; }
                }
            }
            if (heap_id == invalid_index) {
                heap_id = static_cast<std::uint32_t>(plan.heaps.size());
                plan.heaps.push_back({heap_id, req.heap_class, req.compatibility, 0, req.alignment, req.dedicated});
            }
            auto& heap = plan.heaps[heap_id];
            heap.alignment = std::max(heap.alignment, req.alignment);
            std::uint64_t offset{}, capacity{};
            if (!align(heap.bytes, req.alignment, offset) || !align(req.size, req.alignment, capacity) || !add(offset, capacity, heap.bytes))
                return failure(ErrorCode::Overflow, "physical heap offset arithmetic overflow", resource);
            selected = static_cast<std::uint32_t>(slots.size());
            slots.push_back({heap_id, offset, capacity, resource, life.last, 1});
        }
        const auto& slot = slots[selected];
        plan.resources[resource.value] = PhysicalAllocation{slot.heap, selected, slot.offset, req.size, req.alignment, false, predecessor};
    }
    for (auto& heap : plan.heaps) {
        if (!align(heap.bytes, heap.alignment, heap.bytes) || !add(plan.physical_bytes, heap.bytes, plan.physical_bytes))
            return failure(ErrorCode::Overflow, "physical heap total arithmetic overflow");
    }
    for (auto& allocation : plan.resources) if (allocation) allocation->reused_region = slots[allocation->slot].activations > 1;
    if (plan.committed_bytes >= plan.physical_bytes) plan.saved_bytes = plan.committed_bytes - plan.physical_bytes;
    else plan.padding_overhead_bytes = plan.physical_bytes - plan.committed_bytes;
    return plan;
}
}
