#include "framegraph/plan.hpp"
#include <charconv>
#include <cstddef>
#include <iostream>
#include <random>

namespace {
struct Bytes {
    const std::uint8_t* data;
    std::size_t size, position{};
    std::uint8_t next() { return position < size ? data[position++] : 0; }
};
}
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    using namespace framegraph;
    Bytes bytes{data, size};
    GraphDescription g;
    const unsigned resources = 1 + bytes.next() % 8;
    const unsigned passes = 1 + bytes.next() % 16;
    for (unsigned i = 0; i < resources; ++i) {
        ResourceDescription r; r.name = "r" + std::to_string(i);
        if (bytes.next() % 2) r.descriptor = BufferDesc{static_cast<std::uint64_t>(bytes.next()) * bytes.next(), bytes.next() % 2 != 0};
        else r.descriptor = TextureDesc{bytes.next(), bytes.next(), static_cast<Format>(bytes.next() % 5), bytes.next() % 2 != 0, bytes.next() % 2 != 0, bytes.next() % 2 != 0};
        r.imported = bytes.next() % 2 != 0; r.initialized = r.imported && bytes.next() % 2;
        r.exported = bytes.next() % 2 != 0;
        if (r.imported) { r.initial_state = static_cast<ResourceState>(bytes.next() % 12); r.final_state = static_cast<ResourceState>(bytes.next() % 12); }
        g.resources.push_back(r);
    }
    for (unsigned i = 0; i < passes; ++i) {
        PassDescription p; p.name = "p" + std::to_string(i); p.side_effect = bytes.next() % 2 != 0;
        for (unsigned j = 0, n = bytes.next() % 5; j < n; ++j)
            p.usages.push_back({ResourceId{static_cast<std::uint32_t>(bytes.next() % (resources + 2))}, static_cast<ResourceAccess>(bytes.next() % 5), static_cast<Usage>(bytes.next() % 11)});
        g.passes.push_back(p);
    }
    for (unsigned i = 0, n = bytes.next() % 8; i < n; ++i)
        g.ordering.push_back({PassId{static_cast<std::uint32_t>(bytes.next() % (passes + 1))}, PassId{static_cast<std::uint32_t>(bytes.next() % (passes + 1))}});
    std::vector<MemoryRequirement> req;
    for (unsigned i = 0; i < resources; ++i)
        req.push_back({std::uint64_t{1} << (bytes.next() % 64), std::uint64_t{1} << (bytes.next() % 64), static_cast<HeapClass>(bytes.next() % 4), bytes.next(), bytes.next() % 2 != 0});
    const auto plan = PlanCompiler::compile(g, req, bytes.next() % 2 != 0);
    if (plan) { (void)canonical_json(*plan); (void)plan_identity(*plan); }
    (void)json_quote(std::string(reinterpret_cast<const char*>(data), size));
    return 0;
}
#ifndef FRAMEGRAPH_LIBFUZZER
int main(int argc, char** argv) {
    unsigned count = 2000;
    if (argc == 3 && std::string(argv[1]) == "--iterations") {
        const std::string s = argv[2];
        const auto [end, error] = std::from_chars(s.data(), s.data() + s.size(), count);
        if (error != std::errc{} || end != s.data() + s.size() || !count || count > 1000000) return 2;
    } else if (argc != 1) return 2;
    std::mt19937 random(0xD312F022);
    std::vector<std::uint8_t> input(512);
    for (unsigned i = 0; i < count; ++i) {
        for (auto& value : input) value = static_cast<std::uint8_t>(random());
        LLVMFuzzerTestOneInput(input.data(), random() % (input.size() + 1));
    }
    std::cout << "bounded mutation fuzz seed=0xD312F022 iterations=" << count << " passed\n";
}
#endif
