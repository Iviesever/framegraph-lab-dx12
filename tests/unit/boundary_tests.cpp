#include "test_support.hpp"
#include <algorithm>
#include <set>
using namespace framegraph;

CASE(invalid_format_and_states) {
    GraphDescription g; auto bad = test::texture(); std::get<TextureDesc>(bad.descriptor).format = static_cast<Format>(255); g.resources.push_back(bad);
    test::error(g, ErrorCode::InvalidDescriptor);
    g.resources[0] = test::texture(); g.resources[0].imported = g.resources[0].initialized = true; g.resources[0].initial_state = static_cast<ResourceState>(255);
    test::error(g, ErrorCode::InvalidDescriptor);
    g.resources[0].initial_state = ResourceState::ShaderRead; g.resources[0].final_state = ResourceState::DepthWrite;
    test::error(g, ErrorCode::InvalidDescriptor);
}
CASE(imported_war_and_uav_chain) {
    GraphDescription g; auto r = test::texture(); r.imported = r.initialized = true; r.initial_state = ResourceState::ShaderRead; g.resources.push_back(r);
    g.passes = {{"read", {test::read(0)}, true}, {"write", {test::write(0)}, true}};
    const auto a = test::compile(g); CHECK(a.dependencies == std::vector<DependencyEdge>{{PassId{0}, PassId{1}, ResourceId{0}, Hazard::War}});
    GraphDescription u; ResourceDescription buffer{"uav", BufferDesc{256, true}}; buffer.imported = buffer.initialized = true; buffer.initial_state = ResourceState::UnorderedAccess; u.resources.push_back(buffer);
    const auto use = [](ResourceAccess access) { return ResourceUsage{ResourceId{0}, access, Usage::UnorderedAccess}; };
    u.passes = {{"rw0", {use(ResourceAccess::ReadWrite)}, true}, {"r1", {use(ResourceAccess::Read)}, true}, {"rw2", {use(ResourceAccess::ReadWrite)}, true}};
    const auto c = test::compile(u); CHECK(c.dependencies.size() == 4);
    CHECK(std::ranges::find(c.dependencies, DependencyEdge{PassId{0}, PassId{1}, ResourceId{0}, Hazard::Raw}) != c.dependencies.end());
    CHECK(std::ranges::find(c.dependencies, DependencyEdge{PassId{0}, PassId{2}, ResourceId{0}, Hazard::Waw}) != c.dependencies.end());
    CHECK(std::ranges::find(c.dependencies, DependencyEdge{PassId{0}, PassId{2}, ResourceId{0}, Hazard::Raw}) != c.dependencies.end());
    CHECK(std::ranges::find(c.dependencies, DependencyEdge{PassId{1}, PassId{2}, ResourceId{0}, Hazard::War}) != c.dependencies.end());
}
CASE(conservative_overwrite_retention) {
    GraphDescription g; g.resources = {test::texture()}; g.resources[0].exported = true;
    g.passes = {{"w0", {test::write(0)}}, {"r1", {test::read(0)}}, {"w2", {test::write(0)}}};
    const auto c = test::compile(g); CHECK(c.passes.size() == 3); CHECK(c.culled.empty());
}
CASE(cycle_pairs_are_real_edges) {
    GraphDescription g; g.passes = {{"a", {}, true}, {"b", {}, true}, {"c", {}, true}, {"d", {}, true}};
    g.ordering = {{PassId{0},PassId{2}}, {PassId{2},PassId{3}}, {PassId{3},PassId{0}}, {PassId{1},PassId{2}}};
    const auto c = GraphCompiler::compile(g); CHECK(!c && c.error().code == ErrorCode::Cycle);
    const std::set<std::pair<unsigned,unsigned>> edges{{0,2},{2,3},{3,0},{1,2}};
    for (std::size_t i=1;i<c.error().cycle.size();++i) CHECK(edges.contains({c.error().cycle[i-1].value,c.error().cycle[i].value}));
}
CASE(exact_usage_limit_and_plus_one) {
    GraphDescription g; for (unsigned r=0;r<32;++r) g.resources.push_back(test::texture("r"+std::to_string(r)));
    for (unsigned p=0;p<2048;++p) { PassDescription pass{"p"+std::to_string(p),{},false}; for(unsigned r=0;r<32;++r) pass.usages.push_back(test::write(r)); g.passes.push_back(std::move(pass)); }
    CHECK(GraphCompiler::compile(g));
    g.passes.back().usages.push_back(test::write(0)); test::error(g, ErrorCode::LimitExceeded);
}
CASE(exact_edge_limit_and_derived_plus_one) {
    GraphDescription g; g.passes.reserve(max_passes); for(unsigned p=0;p<max_passes;++p) g.passes.push_back({"p"+std::to_string(p),{},p==max_passes-1});
    for(unsigned a=0;a<max_passes && g.ordering.size()<max_edges;++a)
        for(unsigned b=a+1;b<max_passes && g.ordering.size()<max_edges;++b) g.ordering.push_back({PassId{a},PassId{b}});
    CHECK(g.ordering.size()==max_edges); CHECK(GraphCompiler::compile(g));
    g.resources.push_back(test::texture()); g.passes[0].usages={test::write(0)}; g.passes[1].usages={test::read(0)};
    test::error(g, ErrorCode::LimitExceeded);
}
int main(){return test::run();}
