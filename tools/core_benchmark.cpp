#include "framegraph/plan.hpp"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
using namespace framegraph;

constexpr std::uint32_t resource_count = 64;

GraphDescription fixed_graph() {
    GraphDescription graph;
    graph.resources.reserve(resource_count);
    graph.passes.reserve(resource_count * 3);
    for (std::uint32_t id = 0; id < resource_count; ++id) {
        graph.resources.push_back({"buffer-" + std::to_string(id), BufferDesc{512 * 1024, true}});
        const ResourceId resource{id};
        graph.passes.push_back({"write-" + std::to_string(id), {{resource, ResourceAccess::Write, Usage::UnorderedAccess}}});
        graph.passes.push_back({"update-" + std::to_string(id), {{resource, ResourceAccess::ReadWrite, Usage::UnorderedAccess}}});
        graph.passes.push_back({"consume-" + std::to_string(id), {{resource, ResourceAccess::Read, Usage::ShaderRead}}, true});
    }
    return graph;
}

std::vector<MemoryRequirement> fixed_requirements() {
    std::vector<MemoryRequirement> requirements;
    requirements.reserve(resource_count);
    for (std::uint32_t id = 0; id < resource_count; ++id)
        requirements.push_back({65536ull * (1 + id % 8), 65536, HeapClass::Buffer, id % 4, false});
    return requirements;
}

std::uint32_t parse_count(std::string_view value, std::uint32_t maximum) {
    std::uint32_t result{};
    const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), result);
    if (error != std::errc{} || end != value.data() + value.size() || !result || result > maximum)
        throw std::runtime_error("benchmark count is outside its bounded range");
    return result;
}

struct Measurement {
    std::vector<double> raw_us;
    double median_us{};
};

template<class Function>
Measurement measure(std::uint32_t samples, std::uint32_t iterations, std::uint64_t& checksum, Function&& function) {
    const auto warmup = std::min<std::uint32_t>(iterations, 100);
    for (std::uint32_t i = 0; i < warmup; ++i) checksum = (checksum * 1099511628211ull) ^ function();
    Measurement result;
    result.raw_us.reserve(samples);
    for (std::uint32_t sample = 0; sample < samples; ++sample) {
        const auto start = std::chrono::steady_clock::now();
        for (std::uint32_t i = 0; i < iterations; ++i) checksum = (checksum * 1099511628211ull) ^ function();
        const auto elapsed = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - start).count();
        result.raw_us.push_back(elapsed / iterations);
    }
    auto ordered = result.raw_us;
    std::sort(ordered.begin(), ordered.end());
    result.median_us = ordered[ordered.size() / 2];
    return result;
}

std::string compiler_name() {
#if defined(_MSC_VER)
    return "MSVC " + std::to_string(_MSC_VER);
#elif defined(__clang__)
    return "Clang " + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__);
#elif defined(__GNUC__)
    return "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__);
#else
    return "unknown";
#endif
}

void write_measurement(std::ostream& output, const Measurement& measurement) {
    output << "{\"raw_us\":[";
    for (std::size_t i = 0; i < measurement.raw_us.size(); ++i) {
        if (i) output << ',';
        output << measurement.raw_us[i];
    }
    output << "],\"median_us\":" << measurement.median_us << '}';
}

void write_plan(std::ostream& output, const AllocationPlan& allocation, const ResourceStatePlan& barriers) {
    output << "{\"committed_bytes\":" << allocation.committed_bytes
        << ",\"physical_bytes\":" << allocation.physical_bytes
        << ",\"saved_bytes\":" << allocation.saved_bytes
        << ",\"heaps\":" << allocation.heaps.size()
        << ",\"aliases\":" << allocation.aliases.size()
        << ",\"transitions\":" << barriers.transition_count
        << ",\"uav_barriers\":" << barriers.uav_count
        << ",\"aliasing_barriers\":" << barriers.aliasing_count << '}';
}
}

int main(int argc, char** argv) {
    try {
        std::uint32_t samples = 31, iterations = 1000;
        for (int i = 1; i < argc; ++i) {
            const std::string_view key = argv[i];
            if (++i == argc) throw std::runtime_error("missing benchmark option value");
            if (key == "--samples") samples = parse_count(argv[i], 101);
            else if (key == "--iterations") iterations = parse_count(argv[i], 100000);
            else throw std::runtime_error("unknown benchmark option");
        }
        const auto graph = fixed_graph();
        const auto requirements = fixed_requirements();
        const auto compiled_result = GraphCompiler::compile(graph);
        if (!compiled_result) throw std::runtime_error(compiled_result.error().message);
        const auto compiled = *compiled_result;
        const auto allocation_on_result = TransientAllocator::plan(compiled, requirements, true);
        const auto allocation_off_result = TransientAllocator::plan(compiled, requirements, false);
        if (!allocation_on_result || !allocation_off_result) throw std::runtime_error("fixed allocation failed");
        const auto allocation_on = *allocation_on_result;
        const auto allocation_off = *allocation_off_result;
        const auto barriers_on_result = ResourceStatePlanner::plan(compiled, allocation_on);
        const auto barriers_off_result = ResourceStatePlanner::plan(compiled, allocation_off);
        if (!barriers_on_result || !barriers_off_result) throw std::runtime_error("fixed barrier planning failed");
        const auto barriers_on = *barriers_on_result;
        const auto barriers_off = *barriers_off_result;
        const CompiledPlan plan{compiled, allocation_on, barriers_on};

        std::uint64_t checksum = 14695981039346656037ull;
        const auto compile = measure(samples, iterations, checksum, [&] {
            const auto value = GraphCompiler::compile(graph);
            if (!value) throw std::runtime_error(value.error().message);
            return static_cast<std::uint64_t>(value->passes.size() + value->dependencies.size());
        });
        const auto allocation_alias_on = measure(samples, iterations, checksum, [&] {
            const auto value = TransientAllocator::plan(compiled, requirements, true);
            if (!value) throw std::runtime_error(value.error().message);
            return value->physical_bytes + value->aliases.size();
        });
        const auto allocation_alias_off = measure(samples, iterations, checksum, [&] {
            const auto value = TransientAllocator::plan(compiled, requirements, false);
            if (!value) throw std::runtime_error(value.error().message);
            return value->physical_bytes + value->heaps.size();
        });
        const auto barrier_alias_on = measure(samples, iterations, checksum, [&] {
            const auto value = ResourceStatePlanner::plan(compiled, allocation_on);
            if (!value) throw std::runtime_error(value.error().message);
            return static_cast<std::uint64_t>(value->transition_count + value->uav_count + value->aliasing_count);
        });
        const auto barrier_alias_off = measure(samples, iterations, checksum, [&] {
            const auto value = ResourceStatePlanner::plan(compiled, allocation_off);
            if (!value) throw std::runtime_error(value.error().message);
            return static_cast<std::uint64_t>(value->transition_count + value->uav_count + value->aliasing_count);
        });

        std::uint32_t usage_count{};
        for (const auto& pass : graph.passes) usage_count += static_cast<std::uint32_t>(pass.usages.size());
        std::cout << std::fixed << std::setprecision(6)
            << "{\"schema_version\":1,\"compiler\":" << json_quote(compiler_name())
#ifdef NDEBUG
            << ",\"configuration\":\"Release\""
#else
            << ",\"configuration\":\"Debug\""
#endif
            << ",\"graph\":{\"resources\":" << graph.resources.size() << ",\"passes\":" << graph.passes.size()
            << ",\"usages\":" << usage_count << "},\"plan_identity\":" << json_quote(plan_identity(plan))
            << ",\"samples\":" << samples << ",\"iterations_per_sample\":" << iterations << ",\"measurements\":{";
        std::cout << "\"compile\":"; write_measurement(std::cout, compile);
        std::cout << ",\"allocation_alias_on\":"; write_measurement(std::cout, allocation_alias_on);
        std::cout << ",\"allocation_alias_off\":"; write_measurement(std::cout, allocation_alias_off);
        std::cout << ",\"barriers_alias_on\":"; write_measurement(std::cout, barrier_alias_on);
        std::cout << ",\"barriers_alias_off\":"; write_measurement(std::cout, barrier_alias_off);
        std::cout << "},\"plans\":{\"alias_on\":"; write_plan(std::cout, allocation_on, barriers_on);
        std::cout << ",\"alias_off\":"; write_plan(std::cout, allocation_off, barriers_off);
        std::cout << "},\"checksum\":" << checksum << "}\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 2;
    }
}
