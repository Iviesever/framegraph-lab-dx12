#pragma once
#include "d3d12/executor.hpp"
namespace fgl {
struct ProbeProgram {
    framegraph::GraphDescription graph;
    std::vector<PassCallback> callbacks;
    framegraph::ResourceId backbuffer, readback, cull_readback;
};
ProbeProgram make_probe_program(std::uint32_t width, std::uint32_t height, std::uint64_t readback_bytes, std::uint32_t seed);
}
