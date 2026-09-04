#include "probe_graph.hpp"
namespace fgl {
using namespace framegraph;
ProbeProgram make_probe_program(std::uint32_t width, std::uint32_t height, std::uint64_t readback_bytes, std::uint32_t seed) {
    ProbeProgram p; p.graph.scene_seed = seed;
    const TextureDesc texture{width, height, Format::Rgba8, false, false, true};
    const TextureDesc backbuffer_desc{width, height, Format::Rgba8, true, false, false};
    p.graph.resources = {{"ProbeA", texture}, {"ObservedA", texture}, {"ProbeB", texture}, {"StatsUav", BufferDesc{1024, true}},
        {"Backbuffer", backbuffer_desc, true, true, false, ResourceState::Present, ResourceState::Present},
        {"Readback", BufferDesc{readback_bytes}, true, false, false, ResourceState::CopyDest, ResourceState::CopyDest}};
    p.backbuffer = ResourceId{4}; p.readback = ResourceId{5};
    p.graph.passes = {
        {"ClearProbeA", {{ResourceId{0}, ResourceAccess::Write, Usage::UnorderedAccess}}},
        {"ObserveProbeA", {{ResourceId{0}, ResourceAccess::Read, Usage::CopySource}, {ResourceId{1}, ResourceAccess::Write, Usage::CopyDest}}, true},
        {"InitStatsUav", {{ResourceId{3}, ResourceAccess::Write, Usage::UnorderedAccess}}},
        {"UpdateStatsUav", {{ResourceId{3}, ResourceAccess::Write, Usage::UnorderedAccess}}, true},
        {"ClearProbeB", {{ResourceId{2}, ResourceAccess::Write, Usage::UnorderedAccess}}},
        {"BlitProbeB", {{ResourceId{2}, ResourceAccess::Read, Usage::CopySource}, {p.backbuffer, ResourceAccess::Write, Usage::CopyDest}}},
        {"Capture", {{p.backbuffer, ResourceAccess::Read, Usage::CopySource}, {p.readback, ResourceAccess::Write, Usage::CopyDest}}, true},
        {"Present", {{p.backbuffer, ResourceAccess::Read, Usage::Present}}}
    };
    p.graph.ordering.push_back({PassId{6}, PassId{7}});
    p.callbacks.resize(p.graph.passes.size());
    p.callbacks[0] = [](Dx12PassContext& ctx) { ctx.clear_uav_float(ResourceId{0}, {0.85f, 0.08f, 0.2f, 1.f}); };
    p.callbacks[1] = [](Dx12PassContext& ctx) { ctx.copy(ResourceId{0}, ResourceId{1}); };
    p.callbacks[2] = [](Dx12PassContext& ctx) { ctx.clear_uav(ResourceId{3}, {0, 0, 0, 0}); };
    p.callbacks[3] = [seed](Dx12PassContext& ctx) { ctx.clear_uav(ResourceId{3}, {seed, 1, 0, 0}); };
    const float blue = 0.55f + static_cast<float>(seed % 97) / 400.f;
    p.callbacks[4] = [blue](Dx12PassContext& ctx) { ctx.clear_uav_float(ResourceId{2}, {0.08f, 0.32f, blue, 1.f}); };
    p.callbacks[5] = [backbuffer = p.backbuffer](Dx12PassContext& ctx) { ctx.copy(ResourceId{2}, backbuffer); };
    p.callbacks[6] = [backbuffer = p.backbuffer, readback = p.readback](Dx12PassContext& ctx) { ctx.readback(backbuffer, readback); };
    p.callbacks[7] = [backbuffer = p.backbuffer](Dx12PassContext& ctx) { (void)ctx.resource(backbuffer); };
    return p;
}
}
