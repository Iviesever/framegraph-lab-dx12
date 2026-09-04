#pragma once
#include "framegraph/graph.hpp"

namespace framegraph {
enum class HeapClass : std::uint8_t { Buffer, RtDsTexture, Texture };
struct MemoryRequirement {
    std::uint64_t size{}, alignment{65536};
    HeapClass heap_class{HeapClass::RtDsTexture};
    std::uint64_t compatibility{};
    bool dedicated{};
};
struct PhysicalHeap {
    std::uint32_t id{};
    HeapClass heap_class{};
    std::uint64_t compatibility{}, bytes{}, alignment{};
    bool dedicated{};
};
struct PhysicalAllocation {
    std::uint32_t heap{}, slot{};
    std::uint64_t offset{}, size{}, alignment{};
    bool reused_region{};
    ResourceId predecessor;
};
struct AliasingEvent {
    std::uint32_t position{}, heap{};
    std::uint64_t offset{}, size{};
    ResourceId before, after;
};
struct AllocationPlan {
    bool aliasing_enabled{true};
    std::vector<PhysicalHeap> heaps;
    std::vector<std::optional<PhysicalAllocation>> resources;
    std::vector<AliasingEvent> aliases;
    std::uint64_t committed_bytes{}, physical_bytes{}, saved_bytes{}, padding_overhead_bytes{};
};
enum class BarrierKind : std::uint8_t { Transition, Uav, Aliasing };
struct Barrier {
    BarrierKind kind{BarrierKind::Transition};
    ResourceId resource;
    ResourceState before{ResourceState::Common}, after{ResourceState::Common};
    ResourceId alias_before;
    auto operator<=>(const Barrier&) const = default;
};
struct PassBarriers { std::vector<Barrier> before, after; };
struct ResourceStatePlan {
    std::vector<PassBarriers> passes;
    std::vector<Barrier> final;
    std::uint32_t transition_count{}, uav_count{}, aliasing_count{};
};
struct CompiledPlan {
    CompiledGraph graph;
    AllocationPlan allocation;
    ResourceStatePlan barriers;
};
class TransientAllocator {
public:
    static Result<AllocationPlan> plan(const CompiledGraph& graph, const std::vector<MemoryRequirement>& requirements, bool aliasing = true);
};
class ResourceStatePlanner {
public:
    static Result<ResourceStatePlan> plan(const CompiledGraph& graph, const AllocationPlan& allocation);
};
class PlanCompiler {
public:
    static Result<CompiledPlan> compile(const GraphDescription& graph, const std::vector<MemoryRequirement>& requirements, bool aliasing = true);
};
std::string canonical_json(const CompiledPlan& plan);
std::string plan_identity(const CompiledPlan& plan);
} // namespace framegraph
