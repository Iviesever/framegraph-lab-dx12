#include "test_support.hpp"
#include <algorithm>

using namespace framegraph;

CASE(empty_graph) { CHECK(test::compile({}).passes.empty()); }
CASE(invalid_resource_handle) {
    GraphDescription g; g.passes.push_back({"bad", {test::read(99)}, true});
    test::error(g, ErrorCode::InvalidHandle);
}
CASE(invalid_pass_handle) {
    GraphDescription g; g.ordering.push_back({PassId{0}, PassId{1}});
    test::error(g, ErrorCode::InvalidHandle);
}
CASE(zero_texture_dimension) {
    GraphDescription g; auto r = test::texture(); std::get<TextureDesc>(r.descriptor).width = 0;
    g.resources.push_back(r); test::error(g, ErrorCode::InvalidDescriptor);
}
CASE(oversized_texture) {
    GraphDescription g; auto r = test::texture(); std::get<TextureDesc>(r.descriptor).height = 16385;
    g.resources.push_back(r); test::error(g, ErrorCode::InvalidDescriptor);
}
CASE(invalid_texture_flags) {
    GraphDescription g; auto r = test::texture(); std::get<TextureDesc>(r.descriptor).depth_stencil = true;
    g.resources.push_back(r); test::error(g, ErrorCode::InvalidDescriptor);
}
CASE(zero_buffer) {
    GraphDescription g; g.resources.push_back({"buffer", BufferDesc{0}}); test::error(g, ErrorCode::InvalidDescriptor);
}
CASE(extreme_buffer) {
    GraphDescription g; g.resources.push_back({"buffer", BufferDesc{UINT64_MAX}}); test::error(g, ErrorCode::Overflow);
}
CASE(resource_limit) {
    GraphDescription g; g.resources.resize(max_resources + 1); test::error(g, ErrorCode::LimitExceeded);
}
CASE(pass_limit) {
    GraphDescription g; g.passes.resize(max_passes + 1); test::error(g, ErrorCode::LimitExceeded);
}
CASE(name_limit) {
    GraphDescription g; g.resources.push_back(test::texture(std::string(257, 'x'))); test::error(g, ErrorCode::LimitExceeded);
}
CASE(duplicate_usage) {
    GraphDescription g; g.resources.push_back(test::texture()); g.passes.push_back({"dup", {test::write(0), test::write(0)}, true});
    test::error(g, ErrorCode::DuplicateUsage);
}
CASE(conflicting_usage) {
    GraphDescription g; g.resources.push_back(test::texture()); g.passes.push_back({"conflict", {test::write(0), test::read(0)}, true});
    test::error(g, ErrorCode::DuplicateUsage);
}
CASE(illegal_access) {
    GraphDescription g; g.resources.push_back(test::texture()); auto use = test::write(0); use.access = ResourceAccess::Read;
    g.passes.push_back({"bad", {use}, true}); test::error(g, ErrorCode::InvalidUsage);
}
CASE(unknown_usage) {
    GraphDescription g; g.resources.push_back(test::texture()); auto use = test::write(0); use.usage = static_cast<Usage>(255);
    g.passes.push_back({"bad", {use}, true}); test::error(g, ErrorCode::InvalidUsage);
}
CASE(unknown_access) {
    GraphDescription g; g.resources.push_back(test::texture()); auto use = test::write(0); use.access = static_cast<ResourceAccess>(255);
    g.passes.push_back({"bad", {use}, true}); test::error(g, ErrorCode::InvalidUsage);
}
CASE(buffer_cannot_be_render_target) {
    GraphDescription g; g.resources.push_back({"buffer", BufferDesc{64}}); g.passes.push_back({"bad", {test::write(0)}, true});
    test::error(g, ErrorCode::InvalidUsage);
}
CASE(missing_uav_capability) {
    GraphDescription g; g.resources.push_back({"buffer", BufferDesc{64}});
    g.passes.push_back({"bad", {{ResourceId{0}, ResourceAccess::Write, Usage::UnorderedAccess}}, true}); test::error(g, ErrorCode::InvalidUsage);
}
CASE(transient_read_without_producer) {
    GraphDescription g; g.resources.push_back(test::texture()); g.passes.push_back({"read", {test::read(0)}, true});
    test::error(g, ErrorCode::UninitializedRead);
}
CASE(invalid_dead_pass_still_rejected) {
    GraphDescription g; g.resources.push_back(test::texture()); g.passes.push_back({"dead", {test::read(0)}});
    test::error(g, ErrorCode::UninitializedRead);
}
CASE(imported_read) {
    GraphDescription g; auto r = test::texture(); r.imported = r.initialized = true; r.initial_state = ResourceState::ShaderRead;
    g.resources.push_back(r); g.passes.push_back({"read", {test::read(0)}, true}); CHECK(test::compile(g).passes.size() == 1);
}
CASE(uninitialized_import_read) {
    GraphDescription g; auto r = test::texture(); r.imported = true;
    g.resources.push_back(r); g.passes.push_back({"read", {test::read(0)}, true}); test::error(g, ErrorCode::UninitializedRead);
}
CASE(transient_initial_state_must_be_common) {
    GraphDescription g; auto r = test::texture(); r.initial_state = ResourceState::ShaderRead;
    g.resources.push_back(r); test::error(g, ErrorCode::InvalidDescriptor);
}
CASE(unwritten_export) {
    GraphDescription g; auto r = test::texture(); r.exported = true; g.resources.push_back(r); test::error(g, ErrorCode::UninitializedRead);
}
CASE(builder_handles_and_bounds) {
    GraphBuilder b; const auto r = b.add_resource(test::texture()); CHECK(r && r->value == 0);
    const auto p = b.add_pass({"write", {test::write(0)}, true}); CHECK(p && p->value == 0);
    CHECK(!b.order(*p, PassId{5})); CHECK(test::compile(b.description()).passes.size() == 1);
}
CASE(raw_war_waw_edges) {
    GraphDescription g; g.resources.push_back(test::texture());
    g.passes = {{"w0", {test::write(0)}, true}, {"r1", {test::read(0)}, true},
                {"r2", {test::read(0)}, true}, {"w3", {test::write(0)}, true}, {"r4", {test::read(0)}, true}};
    const auto c = test::compile(g);
    const std::vector<DependencyEdge> expected{
        {PassId{0}, PassId{1}, ResourceId{0}, Hazard::Raw}, {PassId{0}, PassId{2}, ResourceId{0}, Hazard::Raw},
        {PassId{0}, PassId{3}, ResourceId{0}, Hazard::Waw}, {PassId{1}, PassId{3}, ResourceId{0}, Hazard::War},
        {PassId{2}, PassId{3}, ResourceId{0}, Hazard::War}, {PassId{3}, PassId{4}, ResourceId{0}, Hazard::Raw}};
    CHECK(c.dependencies == expected);
}
CASE(stable_ready_order) {
    GraphDescription g; g.passes = {{"late", {}, true}, {"independent", {}, true}, {"first", {}, true}};
    g.ordering.push_back({PassId{2}, PassId{0}}); const auto c = test::compile(g);
    CHECK(c.passes.size() == 3); CHECK(c.passes[0].id == PassId{1}); CHECK(c.passes[1].id == PassId{2}); CHECK(c.passes[2].id == PassId{0});
}
CASE(cycle_diagnostic) {
    GraphDescription g; g.passes = {{"A", {}, true}, {"B", {}, true}, {"C", {}, true}};
    g.ordering = {{PassId{0}, PassId{1}}, {PassId{1}, PassId{2}}, {PassId{2}, PassId{0}}};
    const auto c = GraphCompiler::compile(g); CHECK(!c); CHECK(c.error().code == ErrorCode::Cycle);
    CHECK(c.error().cycle.size() == 4); CHECK(c.error().cycle.front() == c.error().cycle.back());
    CHECK(c.error().message.find("A") != std::string::npos);
}
CASE(self_cycle) {
    GraphDescription g; g.passes = {{"A", {}, true}}; g.ordering = {{PassId{0}, PassId{0}}};
    test::error(g, ErrorCode::Cycle);
}
CASE(dead_cycle_rejected) {
    GraphDescription g; g.passes = {{"A", {}, false}, {"B", {}, false}};
    g.ordering = {{PassId{0}, PassId{1}}, {PassId{1}, PassId{0}}}; test::error(g, ErrorCode::Cycle);
}
CASE(cull_and_lifetime) {
    GraphDescription g; g.resources = {test::texture("live"), test::texture("dead")};
    g.passes = {{"producer", {test::write(0)}}, {"dead", {test::write(1)}}, {"consumer", {test::read(0)}, true}};
    const auto c = test::compile(g); CHECK(c.passes.size() == 2); CHECK(c.culled == std::vector<PassId>{PassId{1}});
    CHECK(c.lifetimes.size() == 2); CHECK(c.lifetimes[0] == ResourceLifetime{0, 1}); CHECK(!c.lifetimes[1]);
}
CASE(export_root) {
    GraphDescription g; g.resources = {test::texture()}; g.resources[0].exported = true;
    g.passes = {{"producer", {test::write(0)}}, {"unused", {}}}; const auto c = test::compile(g);
    CHECK(c.passes.size() == 1); CHECK(c.culled == std::vector<PassId>{PassId{1}});
}
CASE(present_root_and_import_final) {
    GraphDescription g; auto r = test::texture("backbuffer"); std::get<TextureDesc>(r.descriptor).format = Format::Rgba8;
    r.imported = r.initialized = true; r.initial_state = ResourceState::Present; r.final_state = ResourceState::Present; g.resources.push_back(r);
    g.passes = {{"tone", {test::write(0)}}, {"present", {{ResourceId{0}, ResourceAccess::Read, Usage::Present}}}, {"unused", {}}};
    const auto c = test::compile(g); CHECK(c.passes.size() == 2); CHECK(c.description.resources[0].final_state == ResourceState::Present);
}
CASE(no_roots_culls_all) {
    GraphDescription g; g.resources = {test::texture()}; g.passes = {{"dead", {test::write(0)}}};
    const auto c = test::compile(g); CHECK(c.passes.empty()); CHECK(c.culled.size() == 1);
}
CASE(readwrite_uav_hazards) {
    GraphDescription g; g.resources = {{"buffer", BufferDesc{256, true}}};
    g.passes = {{"init", {{ResourceId{0}, ResourceAccess::Write, Usage::UnorderedAccess}}},
                {"update", {{ResourceId{0}, ResourceAccess::ReadWrite, Usage::UnorderedAccess}}, true}};
    const auto c = test::compile(g); CHECK(c.dependencies.size() == 2);
    CHECK(c.dependencies[0].hazard == Hazard::Raw); CHECK(c.dependencies[1].hazard == Hazard::Waw);
}
CASE(canonical_identity_repeat_and_change) {
    GraphDescription g; g.resources = {test::texture("quoted\"\nresource")}; g.passes = {{"write", {test::write(0)}, true}};
    const auto a = test::compile(g); const auto b = test::compile(g);
    CHECK(canonical_json(a) == canonical_json(b)); CHECK(plan_identity(a).size() == 16);
    CHECK(canonical_json(a).find("quoted\\\"\\nresource") != std::string::npos);
    g.scene_seed++; CHECK(plan_identity(a) != plan_identity(test::compile(g)));
}
CASE(canonical_edge_order) {
    GraphDescription g; g.passes = {{"A", {}, true}, {"B", {}, true}, {"C", {}, true}};
    g.ordering = {{PassId{1}, PassId{2}}, {PassId{0}, PassId{2}}, {PassId{0}, PassId{2}}};
    const auto a = test::compile(g); std::reverse(g.ordering.begin(), g.ordering.end());
    const auto b = test::compile(g); CHECK(a.dependencies.size() == 2); CHECK(canonical_json(a) == canonical_json(b));
}
CASE(canonical_usage_order) {
    GraphDescription g; g.resources = {test::texture("a"), test::texture("b")};
    g.passes = {{"write", {test::write(1), test::write(0)}, true}};
    const auto a = test::compile(g); std::reverse(g.passes[0].usages.begin(), g.passes[0].usages.end());
    CHECK(canonical_json(a) == canonical_json(test::compile(g))); CHECK(a.passes[0].usages.front().resource == ResourceId{0});
}
CASE(hash_and_json_escaping) {
    CHECK(stable_hash("") == "cbf29ce484222325"); CHECK(stable_hash("hello") == "a430d84680aabd0b");
    CHECK(json_quote(std::string("\0\t\\\"", 4)) == "\"\\u0000\\t\\\\\\\"\"");
}
CASE(export_contents_survive_graph_completion) {
    GraphDescription g; g.resources = {test::texture("export"), test::texture("scratch")}; g.resources[0].exported = true;
    g.passes = {{"export-write", {test::write(0)}}, {"scratch-write", {test::write(1)}, true}};
    const auto c = test::compile(g); CHECK(c.lifetimes[0] == ResourceLifetime{0, 1});
}
CASE(valid_utf8_names_round_trip) {
    const std::string name = "\xe4\xb8\xad\xf0\x9f\x94\xa5";
    CHECK(json_quote(name) == '"' + name + '"');
    CHECK(json_quote(std::string("\xff", 1)) == "\"\\u00ff\"");
}
int main() { return test::run(); }
