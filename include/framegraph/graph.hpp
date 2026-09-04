#pragma once

#include <compare>
#include <cstdint>
#include "framegraph/expected.hpp"
#include <limits>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace framegraph {
inline constexpr std::uint32_t invalid_index = std::numeric_limits<std::uint32_t>::max();
inline constexpr std::uint32_t max_resources = 4096;
inline constexpr std::uint32_t max_passes = 4096;
inline constexpr std::uint32_t max_usages = 65536;
inline constexpr std::uint32_t max_edges = 262144;

struct ResourceId {
    std::uint32_t value{invalid_index};
    auto operator<=>(const ResourceId&) const = default;
};
struct PassId {
    std::uint32_t value{invalid_index};
    auto operator<=>(const PassId&) const = default;
};
enum class Format : std::uint8_t { Rgba8, Rgba16Float, D32Float };
enum class ResourceAccess : std::uint8_t { Read, Write, ReadWrite };
enum class Usage : std::uint8_t {
    ShaderRead, RenderTarget, DepthWrite, DepthRead, UnorderedAccess, CopySource, CopyDest, Present
};
enum class ResourceState : std::uint8_t {
    Common, ShaderRead, RenderTarget, DepthWrite, DepthRead, UnorderedAccess, CopySource, CopyDest, Present
};
struct TextureDesc {
    std::uint32_t width{1}, height{1};
    Format format{Format::Rgba8};
    bool render_target{}, depth_stencil{}, unordered_access{};
    auto operator<=>(const TextureDesc&) const = default;
};
struct BufferDesc {
    std::uint64_t bytes{1};
    bool unordered_access{};
    auto operator<=>(const BufferDesc&) const = default;
};
struct ResourceDescription {
    std::string name;
    std::variant<TextureDesc, BufferDesc> descriptor{TextureDesc{}};
    bool imported{}, initialized{}, exported{};
    ResourceState initial_state{ResourceState::Common};
    std::optional<ResourceState> final_state{};
};
struct ResourceUsage {
    ResourceId resource;
    ResourceAccess access{ResourceAccess::Read};
    Usage usage{Usage::ShaderRead};
    auto operator<=>(const ResourceUsage&) const = default;
};
struct PassDescription {
    std::string name;
    std::vector<ResourceUsage> usages;
    bool side_effect{};
};
struct OrderingEdge {
    PassId before, after;
    auto operator<=>(const OrderingEdge&) const = default;
};
struct GraphDescription {
    std::vector<ResourceDescription> resources;
    std::vector<PassDescription> passes;
    std::vector<OrderingEdge> ordering;
    std::uint32_t scene_seed{24301};
};
enum class ErrorCode : std::uint8_t {
    InvalidHandle, InvalidDescriptor, InvalidUsage, DuplicateUsage, UninitializedRead,
    Cycle, LimitExceeded, Overflow, InvalidMemoryRequirement, UndeclaredAccess, Internal
};
struct GraphError {
    ErrorCode code{ErrorCode::Internal};
    std::string message;
    PassId pass;
    ResourceId resource;
    std::vector<PassId> cycle;
};
template<class T> using Result = Expected<T, GraphError>;

enum class Hazard : std::uint8_t { Raw, War, Waw, Explicit };
struct DependencyEdge {
    PassId before, after;
    ResourceId resource;
    Hazard hazard{Hazard::Explicit};
    auto operator<=>(const DependencyEdge&) const = default;
};
struct ResourceLifetime {
    std::uint32_t first{}, last{};
    auto operator<=>(const ResourceLifetime&) const = default;
};
struct CompiledPass {
    PassId id;
    std::vector<ResourceUsage> usages;
};
struct CompiledGraph {
    GraphDescription description;
    std::vector<CompiledPass> passes;
    std::vector<PassId> culled;
    std::vector<DependencyEdge> dependencies;
    std::vector<std::optional<ResourceLifetime>> lifetimes;
};

class GraphBuilder {
public:
    Result<ResourceId> add_resource(ResourceDescription resource);
    Result<PassId> add_pass(PassDescription pass);
    Result<void> order(PassId before, PassId after);
    const GraphDescription& description() const noexcept { return graph_; }
private:
    GraphDescription graph_;
};
class GraphCompiler {
public:
    static Result<CompiledGraph> compile(const GraphDescription& description);
};

std::string canonical_json(const CompiledGraph& graph);
std::string plan_identity(const CompiledGraph& graph);
std::string json_quote(const std::string& text);
std::string stable_hash(const std::string& bytes);
} // namespace framegraph
