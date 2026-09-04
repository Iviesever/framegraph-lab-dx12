#include "test_support.hpp"
#include "framegraph/plan.hpp"
#include "../property/plan_oracle.hpp"

using namespace framegraph;
namespace {
GraphDescription disjoint() {
    GraphDescription g; g.resources = {test::texture("a"), test::texture("b")};
    g.passes = {{"a-write", {test::write(0)}, true}, {"b-write", {test::write(1)}, true}};
    return g;
}
std::vector<MemoryRequirement> memory(std::uint64_t size = 65536) { return {{size}, {size}}; }
CompiledPlan compile(const GraphDescription& g, const std::vector<MemoryRequirement>& req, bool alias = true) {
    auto p = PlanCompiler::compile(g, req, alias);
    if (!p) throw std::runtime_error(p.error().message);
    return std::move(*p);
}
void failure(const std::vector<MemoryRequirement>& req, ErrorCode error) {
    const auto result = PlanCompiler::compile(disjoint(), req); CHECK(!result); CHECK(result.error().code == error);
}
}
CASE(disjoint_lifetimes_share_slot) {
    const auto p = compile(disjoint(), memory()); const auto& a = p.allocation;
    CHECK(a.heaps.size() == 1); CHECK(a.resources[0]->offset == a.resources[1]->offset);
    CHECK(a.committed_bytes == 131072); CHECK(a.physical_bytes == 65536); CHECK(a.saved_bytes == 65536);
    CHECK(a.aliases.size() == 1); CHECK(a.aliases[0].before == ResourceId{0}); CHECK(a.aliases[0].after == ResourceId{1});
    CHECK(a.resources[0]->reused_region); CHECK(a.resources[1]->reused_region);
}
CASE(reference_mode_disjoint_bytes) {
    const auto p = compile(disjoint(), memory(), false); const auto& a = p.allocation;
    CHECK(a.resources[0]->offset != a.resources[1]->offset); CHECK(a.physical_bytes == 131072);
    CHECK(a.saved_bytes == 0); CHECK(a.aliases.empty()); CHECK(p.barriers.aliasing_count == 2);
}
CASE(overlap_cannot_alias) {
    auto g = disjoint(); g.passes.push_back({"read-both", {test::read(0), test::read(1)}, true});
    const auto a = compile(g, memory()).allocation; CHECK(a.resources[0]->offset != a.resources[1]->offset); CHECK(a.aliases.empty());
}
CASE(export_cannot_be_clobbered) {
    auto g = disjoint(); g.resources[0].exported = true;
    const auto a = compile(g, memory()).allocation; CHECK(a.resources[0]->offset != a.resources[1]->offset);
}
CASE(offset_alignment) {
    auto m = memory(); m[0] = {3, 1}; m[1] = {3, 8};
    const auto a = compile(disjoint(), m, false).allocation; CHECK(a.resources[1]->offset == 8); CHECK(a.physical_bytes == 16);
    CHECK(a.padding_overhead_bytes == 5); CHECK(a.saved_bytes == 0);
}
CASE(compatibility_separates_heaps) {
    auto m = memory(); m[1].compatibility = 12;
    const auto a = compile(disjoint(), m).allocation; CHECK(a.heaps.size() == 2); CHECK(a.aliases.empty());
}
CASE(dedicated_never_reused) {
    auto m = memory(); m[0].dedicated = true;
    const auto a = compile(disjoint(), m).allocation; CHECK(a.heaps.size() == 2); CHECK(a.aliases.empty());
}
CASE(size_growth_uses_new_slot) {
    auto m = memory(); m[1].size *= 2;
    const auto a = compile(disjoint(), m).allocation; CHECK(a.resources[0]->slot != a.resources[1]->slot);
}
CASE(heap_classes_are_separate) {
    auto g = disjoint(); g.resources[1].descriptor = BufferDesc{65536, true};
    g.passes[1].usages = {{ResourceId{1}, ResourceAccess::Write, Usage::UnorderedAccess}};
    auto m = memory(); m[1].heap_class = HeapClass::Buffer;
    const auto a = compile(g, m).allocation; CHECK(a.heaps.size() == 2); CHECK(a.aliases.empty());
}
CASE(imports_are_not_allocated) {
    auto g = disjoint(); g.resources[0].imported = true;
    auto m = memory(); m[0].size = 0;
    const auto a = compile(g, m).allocation; CHECK(!a.resources[0]); CHECK(a.physical_bytes == 65536);
}
CASE(unused_transients_are_not_allocated) {
    auto g = disjoint(); g.passes[0].side_effect = false;
    const auto a = compile(g, memory()).allocation; CHECK(!a.resources[0]); CHECK(a.physical_bytes == 65536);
}
CASE(requirement_count_checked) { failure({}, ErrorCode::InvalidMemoryRequirement); }
CASE(zero_size_checked) { auto m = memory(); m[0].size = 0; failure(m, ErrorCode::InvalidMemoryRequirement); }
CASE(zero_alignment_checked) { auto m = memory(); m[0].alignment = 0; failure(m, ErrorCode::InvalidMemoryRequirement); }
CASE(non_power_two_alignment_checked) { auto m = memory(); m[0].alignment = 3; failure(m, ErrorCode::InvalidMemoryRequirement); }
CASE(class_descriptor_mismatch_checked) { auto m = memory(); m[0].heap_class = HeapClass::Buffer; failure(m, ErrorCode::InvalidMemoryRequirement); }
CASE(size_rounding_overflow_checked) { auto m = memory(); m[0].size = UINT64_MAX; failure(m, ErrorCode::Overflow); }
CASE(sum_overflow_checked) { auto m = memory(1ull << 63); failure(m, ErrorCode::Overflow); }
CASE(alias_activation_precedes_transition) {
    const auto p = compile(disjoint(), memory());
    CHECK(p.barriers.passes[0].before[0].kind == BarrierKind::Aliasing);
    CHECK(p.barriers.passes[0].before[0].alias_before == ResourceId{});
    CHECK(p.barriers.passes[1].before[0].alias_before == ResourceId{0});
    CHECK(p.barriers.passes[1].before[1].kind == BarrierKind::Transition);
    CHECK(p.barriers.passes[0].after.back().after == ResourceState::Common);
    CHECK(p.barriers.transition_count == 4); CHECK(p.barriers.aliasing_count == 2);
}
CASE(equal_state_has_no_redundant_transition) {
    auto g = disjoint(); g.passes.insert(g.passes.begin() + 1, {"a-again", {test::write(0)}, true});
    const auto p = compile(g, memory()); CHECK(p.barriers.passes[1].before.empty()); CHECK(p.barriers.transition_count == 4);
}
CASE(uav_write_then_read_orders) {
    GraphDescription g; g.resources = {{"uav", BufferDesc{65536, true}}};
    g.passes = {{"init", {{ResourceId{0}, ResourceAccess::Write, Usage::UnorderedAccess}}},
                {"read", {{ResourceId{0}, ResourceAccess::Read, Usage::UnorderedAccess}}, true}};
    const auto p = compile(g, {{65536, 65536, HeapClass::Buffer}});
    CHECK(p.barriers.uav_count == 1); CHECK(p.barriers.passes[1].before.front().kind == BarrierKind::Uav);
}
CASE(uav_read_read_needs_no_ordering) {
    GraphDescription g; ResourceDescription r{"uav", BufferDesc{65536, true}};
    r.imported = r.initialized = true; r.initial_state = ResourceState::UnorderedAccess; g.resources.push_back(r);
    g.passes = {{"read1", {{ResourceId{0}, ResourceAccess::Read, Usage::UnorderedAccess}}, true},
                {"read2", {{ResourceId{0}, ResourceAccess::Read, Usage::UnorderedAccess}}, true}};
    const auto p = compile(g, {MemoryRequirement{}}); CHECK(p.barriers.uav_count == 0); CHECK(p.barriers.transition_count == 0);
}
CASE(imported_required_final_transition) {
    GraphDescription g; auto r = test::texture(); r.imported = r.initialized = true;
    r.initial_state = ResourceState::ShaderRead; r.final_state = ResourceState::CopySource; g.resources.push_back(r);
    g.passes = {{"write", {test::write(0)}, true}}; const auto p = compile(g, {MemoryRequirement{}});
    CHECK(p.barriers.passes[0].before.front().before == ResourceState::ShaderRead);
    CHECK(p.barriers.final.size() == 1); CHECK(p.barriers.final[0].after == ResourceState::CopySource);
}
CASE(unused_import_final_transition) {
    GraphDescription g; auto r = test::texture(); r.imported = r.initialized = true;
    r.initial_state = ResourceState::ShaderRead; r.final_state = ResourceState::CopySource; g.resources.push_back(r);
    const auto p = compile(g, {MemoryRequirement{}}); CHECK(p.barriers.final.size() == 1);
    CHECK(p.barriers.final[0].before == ResourceState::ShaderRead); CHECK(p.barriers.final[0].after == ResourceState::CopySource);
}
CASE(common_present_are_native_equivalent) {
    GraphDescription g; auto r = test::texture(); r.imported = r.initialized = true;
    std::get<TextureDesc>(r.descriptor).format = Format::Rgba8; r.final_state = ResourceState::Present; g.resources.push_back(r);
    const auto p = compile(g, {MemoryRequirement{}}); CHECK(p.barriers.final.empty());
}
CASE(execution_identity_covers_policy_and_barriers) {
    const auto a = compile(disjoint(), memory()); const auto b = compile(disjoint(), memory());
    CHECK(canonical_json(a) == canonical_json(b)); CHECK(plan_identity(a).size() == 16);
    CHECK(plan_identity(a) != plan_identity(compile(disjoint(), memory(), false)));
    auto changed = a; changed.barriers.passes[0].before.clear(); CHECK(plan_identity(a) != plan_identity(changed));
    CHECK(canonical_json(a).find("\"allocation\"") != std::string::npos);
}
CASE(oracle_rejects_activation_after_transition) {
    auto p = compile(disjoint(), memory());
    std::swap(p.barriers.passes[1].before[0], p.barriers.passes[1].before[1]);
    bool rejected = false;
    try { property::verify_plan(p, memory()); } catch (const std::runtime_error&) { rejected = true; }
    CHECK(rejected);
}
CASE(standalone_placed_resources_activate) {
    const auto p = compile(disjoint(), memory(), false);
    CHECK(p.barriers.passes[0].before.front().kind == BarrierKind::Aliasing);
    CHECK(p.barriers.passes[1].before.front().kind == BarrierKind::Aliasing);
    CHECK(p.barriers.passes[1].before.front().alias_before == ResourceId{});
    property::verify_plan(p, memory());
}
CASE(oracle_rejects_live_byte_overlap) {
    auto g = disjoint(); g.passes.push_back({"read-both", {test::read(0), test::read(1)}, true});
    auto p = compile(g, memory()); p.allocation.resources[1]->offset = p.allocation.resources[0]->offset;
    bool rejected = false;
    try { property::verify_plan(p, memory()); } catch (const std::runtime_error&) { rejected = true; }
    CHECK(rejected);
}
int main() { return test::run(); }
