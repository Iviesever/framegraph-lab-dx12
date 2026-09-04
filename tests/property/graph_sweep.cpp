#include "../unit/test_support.hpp"
#include "plan_oracle.hpp"
#include <algorithm>
#include <charconv>
#include <random>

using namespace framegraph;
namespace {
GraphDescription generate(std::mt19937& rng) {
    GraphDescription g;
    const auto count = 2u + rng() % 7u;
    for (std::uint32_t i = 0; i < count; ++i) {
        auto r = test::texture("r" + std::to_string(i)); r.exported = (rng() % 3 == 0);
        g.resources.push_back(r);
        g.passes.push_back({"initialize" + std::to_string(i), {test::write(i)}});
    }
    const auto extra = 2u + rng() % 12u;
    for (std::uint32_t i = 0; i < extra; ++i) {
        const auto w = static_cast<std::uint32_t>(rng() % count);
        const auto r = static_cast<std::uint32_t>((w + 1 + rng() % (count - 1)) % count);
        std::vector<ResourceUsage> uses{test::write(w)};
        if (rng() % 2) uses.push_back(test::read(r));
        g.passes.push_back({"work" + std::to_string(i), uses, rng() % 4 == 0});
    }
    g.passes.back().side_effect = true;
    // Fresh late resources produce both alias opportunities and partial-size reuse.
    for (unsigned i = 0, n = 1 + rng() % 4; i < n; ++i) {
        const auto r = static_cast<std::uint32_t>(g.resources.size());
        g.resources.push_back(test::texture("scratch" + std::to_string(i)));
        g.passes.push_back({"scratch-pass" + std::to_string(i), {test::write(r)}, true});
    }
    for (unsigned i = 0; i < 5; ++i) {
        auto a = static_cast<std::uint32_t>(rng() % g.passes.size());
        auto b = static_cast<std::uint32_t>(rng() % g.passes.size());
        if (a > b) std::swap(a, b);
        if (a != b) g.ordering.push_back({PassId{a}, PassId{b}});
    }
    return g;
}

std::vector<MemoryRequirement> requirements(const GraphDescription& g, std::mt19937& rng) {
    std::vector<MemoryRequirement> result;
    for (const auto& r : g.resources) {
        const auto* t = std::get_if<TextureDesc>(&r.descriptor);
        const auto type = !t ? HeapClass::Buffer : t->render_target || t->depth_stencil ? HeapClass::RtDsTexture : HeapClass::Texture;
        result.push_back({1024 + rng() % 65536, std::uint64_t{1} << (rng() % 17), type, rng() % 3, rng() % 11 == 0});
    }
    return result;
}

// Deliberately quadratic pairwise oracle: no production hazard scan or topo helpers.
void verify(const GraphDescription& g, const CompiledGraph& c) {
    std::vector<DependencyEdge> expected;
    for (std::uint32_t r = 0; r < g.resources.size(); ++r) {
        std::vector<std::pair<std::uint32_t, ResourceAccess>> uses;
        for (std::uint32_t p = 0; p < g.passes.size(); ++p)
            for (const auto& u : g.passes[p].usages) if (u.resource.value == r) uses.emplace_back(p, u.access);
        for (std::size_t a = 0; a < uses.size(); ++a) for (std::size_t b = a + 1; b < uses.size(); ++b) {
            bool intermediate_write = false;
            for (auto k = a + 1; k < b; ++k) if (uses[k].second != ResourceAccess::Read) intermediate_write = true;
            if (intermediate_write) continue;
            auto add = [&](Hazard h) { expected.push_back({PassId{uses[a].first}, PassId{uses[b].first}, ResourceId{r}, h}); };
            if (uses[a].second != ResourceAccess::Read && uses[b].second != ResourceAccess::Write) add(Hazard::Raw);
            if (uses[a].second != ResourceAccess::Read && uses[b].second != ResourceAccess::Read) add(Hazard::Waw);
            if (uses[a].second == ResourceAccess::Read && uses[b].second != ResourceAccess::Read) add(Hazard::War);
        }
    }
    for (const auto& e : g.ordering) expected.push_back({e.before, e.after, {}, Hazard::Explicit});
    std::sort(expected.begin(), expected.end());
    expected.erase(std::unique(expected.begin(), expected.end()), expected.end());
    std::vector<bool> live(g.passes.size());
    for (std::size_t i = 0; i < g.passes.size(); ++i) {
        live[i] = g.passes[i].side_effect;
        for (const auto& u : g.passes[i].usages) if (u.usage == Usage::Present) live[i] = true;
    }
    for (std::uint32_t r = 0; r < g.resources.size(); ++r) if (g.resources[r].exported) {
        std::uint32_t last = invalid_index;
        for (std::uint32_t p = 0; p < g.passes.size(); ++p)
            for (const auto& u : g.passes[p].usages) if (u.resource.value == r && u.access != ResourceAccess::Read) last = p;
        if (last != invalid_index) live[last] = true;
    }
    for (std::size_t iteration = 0; iteration < g.passes.size(); ++iteration)
        for (const auto& e : expected) if (live[e.after.value]) live[e.before.value] = true;
    std::erase_if(expected, [&](const auto& e) { return !live[e.before.value] || !live[e.after.value]; });
    CHECK(c.dependencies == expected);
    std::vector<bool> emitted(g.passes.size());
    for (const auto& pass : c.passes) {
        std::uint32_t smallest = invalid_index;
        for (std::uint32_t candidate = 0; candidate < g.passes.size(); ++candidate) {
            if (!live[candidate] || emitted[candidate]) continue;
            bool ready = true;
            for (const auto& e : expected) if (e.after.value == candidate && !emitted[e.before.value]) ready = false;
            if (ready) { smallest = candidate; break; }
        }
        CHECK(pass.id.value == smallest); emitted[smallest] = true;
    }
    CHECK(emitted == live);
    CHECK(c.culled.size() + c.passes.size() == g.passes.size());
    CHECK(c.lifetimes.size() == g.resources.size());
    for (std::uint32_t r = 0; r < g.resources.size(); ++r) {
        std::vector<std::uint32_t> positions;
        for (std::uint32_t p = 0; p < c.passes.size(); ++p)
            for (const auto& u : c.passes[p].usages) if (u.resource.value == r) positions.push_back(p);
        if (positions.empty()) CHECK(!c.lifetimes[r]);
        else {
            const auto end = g.resources[r].exported ? static_cast<std::uint32_t>(c.passes.size() - 1) : positions.back();
            CHECK(c.lifetimes[r] == ResourceLifetime{positions.front(), end});
        }
    }
    CHECK(canonical_json(c) == canonical_json(test::compile(g)));
    auto reordered = g;
    std::reverse(reordered.ordering.begin(), reordered.ordering.end());
    for (auto& p : reordered.passes) std::reverse(p.usages.begin(), p.usages.end());
    CHECK(plan_identity(c) == plan_identity(test::compile(reordered)));
}
}
int main(int argc, char** argv) {
    unsigned count = 256;
    if (argc == 3 && std::string(argv[1]) == "--cases") {
        const std::string value = argv[2];
        const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), count);
        if (error != std::errc{} || end != value.data() + value.size() || !count || count > 100000) return 2;
    } else if (argc != 1) return 2;
    std::mt19937 rng(0xF6A123u);
    try {
        for (unsigned i = 0; i < count; ++i) {
            auto g = generate(rng); verify(g, test::compile(g));
            // Vary buffer/texture heap class and UAV/read-write ordering without
            // reusing production conversion helpers in the generator or oracle.
            auto device_graph = g;
            for (std::uint32_t r = 0; r < device_graph.resources.size(); ++r) {
                const auto kind = rng() % 4;
                if (kind == 0 || kind == 2) {
                    if (kind == 0)
                    device_graph.resources[r].descriptor = BufferDesc{65536, true};
                    else device_graph.resources[r].descriptor = TextureDesc{64, 32, Format::Rgba16Float, false, false, true};
                    for (auto& pass : device_graph.passes) for (auto& use : pass.usages) if (use.resource.value == r) use.usage = Usage::UnorderedAccess;
                } else if (kind == 1) {
                    device_graph.resources[r].descriptor = TextureDesc{64, 32, Format::D32Float, false, true, false};
                    for (auto& pass : device_graph.passes) for (auto& use : pass.usages) if (use.resource.value == r)
                        use.usage = use.access == ResourceAccess::Read ? Usage::DepthRead : Usage::DepthWrite;
                }
                if (rng() % 7 == 0) {
                    auto& desc = device_graph.resources[r]; desc.imported = desc.initialized = true;
                    desc.initial_state = ResourceState::CopySource; desc.final_state = ResourceState::CopySource;
                }
            }
            std::vector<bool> initialized(device_graph.resources.size());
            for (auto& pass : device_graph.passes) for (auto& use : pass.usages) {
                if (use.usage == Usage::UnorderedAccess && use.access == ResourceAccess::Write && initialized[use.resource.value] && rng() % 2)
                    use.access = ResourceAccess::ReadWrite;
                if (use.access != ResourceAccess::Read) initialized[use.resource.value] = true;
            }
            const auto req = requirements(device_graph, rng);
            const auto plan = PlanCompiler::compile(device_graph, req); CHECK(plan);
            property::verify_plan(*plan, req);
            const auto reference = PlanCompiler::compile(device_graph, req, false); CHECK(reference);
            property::verify_plan(*reference, req);
            const auto repeat = PlanCompiler::compile(device_graph, req); CHECK(repeat);
            CHECK(canonical_json(*plan) == canonical_json(*repeat));
            auto invalid = g;
            switch (i % 3) {
            case 0:
                invalid.ordering.push_back({PassId{0}, PassId{static_cast<std::uint32_t>(g.passes.size() - 1)}});
                invalid.ordering.push_back({PassId{static_cast<std::uint32_t>(g.passes.size() - 1)}, PassId{0}});
                test::error(invalid, ErrorCode::Cycle); break;
            case 1:
                invalid.passes[0].usages = {test::read(0)};
                test::error(invalid, ErrorCode::UninitializedRead); break;
            default:
                invalid.passes[0].usages[0].usage = Usage::Present;
                test::error(invalid, ErrorCode::InvalidUsage); break;
            }
        }
        std::cout << "seed=0xF6A123 valid=" << count << " invalid=" << count << " dependency/lifetime/allocation/transition/UAV/alias/identity invariants passed\n";
    } catch (const std::exception& e) { std::cerr << "property failure: " << e.what() << '\n'; return 1; }
}
