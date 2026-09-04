#include "validation.hpp"
#include <algorithm>

namespace framegraph::detail {
namespace {
bool state_supported(const ResourceDescription& r, ResourceState state) {
    const auto* t = std::get_if<TextureDesc>(&r.descriptor);
    const bool uav = t ? t->unordered_access : std::get<BufferDesc>(r.descriptor).unordered_access;
    switch (state) {
    case ResourceState::Common: case ResourceState::ShaderRead:
    case ResourceState::CopySource: case ResourceState::CopyDest: return true;
    case ResourceState::RenderTarget: return t && t->render_target;
    case ResourceState::DepthWrite: case ResourceState::DepthRead: return t && t->depth_stencil;
    case ResourceState::UnorderedAccess: return uav;
    case ResourceState::Present: return r.imported && t && t->format == Format::Rgba8;
    }
    return false;
}
bool usage_supported(const ResourceDescription& r, const ResourceUsage& u) {
    if (u.access != ResourceAccess::Read && u.access != ResourceAccess::Write && u.access != ResourceAccess::ReadWrite) return false;
    switch (u.usage) {
    case Usage::ShaderRead: return u.access == ResourceAccess::Read;
    case Usage::RenderTarget: return u.access == ResourceAccess::Write && state_supported(r, ResourceState::RenderTarget);
    case Usage::DepthWrite: return u.access == ResourceAccess::Write && state_supported(r, ResourceState::DepthWrite);
    case Usage::DepthRead: return u.access == ResourceAccess::Read && state_supported(r, ResourceState::DepthRead);
    case Usage::UnorderedAccess: return state_supported(r, ResourceState::UnorderedAccess);
    case Usage::CopySource: return u.access == ResourceAccess::Read;
    case Usage::CopyDest: return u.access == ResourceAccess::Write;
    case Usage::Present: return u.access == ResourceAccess::Read && state_supported(r, ResourceState::Present);
    }
    return false;
}
}

Result<void> validate(const GraphDescription& g) {
    auto failure = [](ErrorCode code, std::string text, PassId p = {}, ResourceId r = {}) -> Result<void> {
        return unexpected(GraphError{code, std::move(text), p, r, {}});
    };
    if (g.resources.size() > max_resources || g.passes.size() > max_passes || g.ordering.size() > max_edges)
        return failure(ErrorCode::LimitExceeded, "graph exceeds bounded resource/pass/edge count");
    for (std::uint32_t i = 0; i < g.resources.size(); ++i) {
        const auto& r = g.resources[i];
        if (r.name.size() > 256) return failure(ErrorCode::LimitExceeded, "resource name exceeds 256 bytes", {}, ResourceId{i});
        if (r.name.empty()) return failure(ErrorCode::InvalidDescriptor, "resource name is empty", {}, ResourceId{i});
        if (const auto* t = std::get_if<TextureDesc>(&r.descriptor)) {
            if (!t->width || !t->height || t->width > 16384 || t->height > 16384)
                return failure(ErrorCode::InvalidDescriptor, "texture dimensions must be in [1,16384]", {}, ResourceId{i});
            if (t->format != Format::Rgba8 && t->format != Format::Rgba16Float && t->format != Format::D32Float)
                return failure(ErrorCode::InvalidDescriptor, "unsupported texture format", {}, ResourceId{i});
            if ((t->format == Format::D32Float) != t->depth_stencil || (t->depth_stencil && (t->render_target || t->unordered_access)))
                return failure(ErrorCode::InvalidDescriptor, "depth format/attachment flags conflict", {}, ResourceId{i});
        } else {
            const auto bytes = std::get<BufferDesc>(r.descriptor).bytes;
            if (!bytes) return failure(ErrorCode::InvalidDescriptor, "buffer size is zero", {}, ResourceId{i});
            if (bytes > (std::uint64_t{1} << 40)) return failure(ErrorCode::Overflow, "buffer exceeds supported 1 TiB bound", {}, ResourceId{i});
        }
        if (!r.imported && (r.initialized || r.initial_state != ResourceState::Common || r.final_state))
            return failure(ErrorCode::InvalidDescriptor, "transient starts uninitialized/Common and cannot request imported final state", {}, ResourceId{i});
        if (!state_supported(r, r.initial_state) || (r.final_state && !state_supported(r, *r.final_state)))
            return failure(ErrorCode::InvalidDescriptor, "descriptor cannot support requested state", {}, ResourceId{i});
    }
    for (const auto& e : g.ordering)
        if (e.before.value >= g.passes.size() || e.after.value >= g.passes.size())
            return failure(ErrorCode::InvalidHandle, "ordering edge references unknown pass");
    std::vector<bool> initialized;
    for (const auto& r : g.resources) initialized.push_back(r.imported && r.initialized);
    std::size_t usage_count = 0;
    for (std::uint32_t i = 0; i < g.passes.size(); ++i) {
        const auto& p = g.passes[i];
        if (p.name.size() > 256) return failure(ErrorCode::LimitExceeded, "pass name exceeds 256 bytes", PassId{i});
        if (p.name.empty()) return failure(ErrorCode::InvalidDescriptor, "pass name is empty", PassId{i});
        if (p.usages.size() > max_usages - usage_count) return failure(ErrorCode::LimitExceeded, "graph exceeds usage limit", PassId{i});
        usage_count += p.usages.size();
        std::vector<bool> seen(g.resources.size());
        for (const auto& u : p.usages) {
            if (u.resource.value >= g.resources.size()) return failure(ErrorCode::InvalidHandle, "usage references unknown resource", PassId{i}, u.resource);
            if (seen[u.resource.value]) return failure(ErrorCode::DuplicateUsage, "one usage per resource per pass; use explicit ReadWrite/UAV", PassId{i}, u.resource);
            seen[u.resource.value] = true;
        }
        for (const auto& u : p.usages) {
            if (!usage_supported(g.resources[u.resource.value], u)) return failure(ErrorCode::InvalidUsage, "unsupported access/usage/descriptor combination", PassId{i}, u.resource);
            if (reads(u.access) && !initialized[u.resource.value]) return failure(ErrorCode::UninitializedRead, "resource read has no initialized contents", PassId{i}, u.resource);
            if (writes(u.access)) initialized[u.resource.value] = true;
        }
    }
    for (std::uint32_t i = 0; i < g.resources.size(); ++i)
        if (g.resources[i].exported && !initialized[i]) return failure(ErrorCode::UninitializedRead, "exported resource has no producer", {}, ResourceId{i});
    return {};
}
} // namespace framegraph::detail
