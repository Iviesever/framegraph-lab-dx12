#pragma once
#include "framegraph/graph.hpp"
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace test {
using Case = std::pair<std::string, std::function<void()>>;
inline std::vector<Case>& cases() { static std::vector<Case> value; return value; }
struct Register { Register(std::string name, std::function<void()> run) { cases().emplace_back(std::move(name), std::move(run)); } };
inline void check(bool ok, const char* expression, int line) {
    if (!ok) throw std::runtime_error(std::string(expression) + " at line " + std::to_string(line));
}
inline int run() {
    unsigned failed = 0;
    for (const auto& [name, fn] : cases()) {
        try { fn(); std::cout << "PASS " << name << '\n'; }
        catch (const std::exception& e) { ++failed; std::cerr << "FAIL " << name << ": " << e.what() << '\n'; }
    }
    std::cout << "cases=" << cases().size() << " passed=" << cases().size() - failed << " failed=" << failed << '\n';
    return failed ? 1 : 0;
}
inline framegraph::ResourceDescription texture(std::string name = "texture") {
    return {std::move(name), framegraph::TextureDesc{64, 32, framegraph::Format::Rgba16Float, true, false, true}};
}
inline framegraph::ResourceUsage write(unsigned r) {
    return {framegraph::ResourceId{r}, framegraph::ResourceAccess::Write, framegraph::Usage::RenderTarget};
}
inline framegraph::ResourceUsage read(unsigned r) {
    return {framegraph::ResourceId{r}, framegraph::ResourceAccess::Read, framegraph::Usage::ShaderRead};
}
inline framegraph::CompiledGraph compile(const framegraph::GraphDescription& g) {
    auto result = framegraph::GraphCompiler::compile(g);
    if (!result) throw std::runtime_error("compile failed: " + result.error().message);
    return std::move(*result);
}
inline void error(const framegraph::GraphDescription& g, framegraph::ErrorCode code) {
    auto result = framegraph::GraphCompiler::compile(g);
    check(!result, "expected rejection", __LINE__);
    check(result.error().code == code, "typed error category", __LINE__);
}
}
#define CHECK(...) test::check(static_cast<bool>((__VA_ARGS__)), #__VA_ARGS__, __LINE__)
#define CASE(name) static void name(); static test::Register reg_##name(#name, name); static void name()
