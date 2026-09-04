#include "framegraph/plan.hpp"
#include <iostream>

int main() {
    using namespace framegraph;
    GraphDescription g;
    g.resources = {{"source\"\n中", TextureDesc{64, 32, Format::Rgba16Float, true}},
                   {"result", TextureDesc{64, 32, Format::Rgba16Float, true}}};
    g.passes = {{"write-source", {{ResourceId{0}, ResourceAccess::Write, Usage::RenderTarget}}},
                {"observe-source", {{ResourceId{0}, ResourceAccess::Read, Usage::ShaderRead}}, true},
                {"write-result", {{ResourceId{1}, ResourceAccess::Write, Usage::RenderTarget}}, true},
                {"unused", {}}};
    auto plan = PlanCompiler::compile(g, {{65536}, {65536}});
    if (!plan) { std::cerr << plan.error().message << '\n'; return 1; }
    std::cout << canonical_json(*plan) << '\n';
}
