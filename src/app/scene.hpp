#pragma once
#include "d3d12/executor.hpp"
#include "d3d12/shader.hpp"
#include <array>
namespace fgl {
struct SceneState { float yaw{0.7f}; std::uint32_t logical_frame{}, debug_view{}, seed{24301}; bool paused{}, single_step{}, gpu_driven{true}; };
class SceneRenderer {
public:
    static constexpr std::uint32_t pillar_count = 160;
    SceneRenderer(ID3D12Device* device, const std::filesystem::path& shader_directory);
    void init_culling(Dx12PassContext& pass, framegraph::ResourceId visible, framegraph::ResourceId arguments);
    void cull(Dx12PassContext& pass, framegraph::ResourceId visible, framegraph::ResourceId arguments, const SceneState& state);
    std::vector<std::uint32_t> visible_instances(const SceneState& state, std::uint32_t width, std::uint32_t height) const;
    void depth(Dx12PassContext& pass, framegraph::ResourceId target, framegraph::ResourceId visible, framegraph::ResourceId arguments, const SceneState& state);
    void scene(Dx12PassContext& pass, framegraph::ResourceId target, framegraph::ResourceId depth, framegraph::ResourceId visible, framegraph::ResourceId arguments, const SceneState& state);
    void extract(Dx12PassContext& pass, framegraph::ResourceId source, framegraph::ResourceId target);
    void blur(Dx12PassContext& pass, framegraph::ResourceId source, framegraph::ResourceId target, float x, float y);
    void tone(Dx12PassContext& pass, framegraph::ResourceId hdr, framegraph::ResourceId bloom, framegraph::ResourceId target, const SceneState& state);
private:
    std::array<std::uint32_t, 32> constants(std::uint32_t width, std::uint32_t height, const SceneState& state) const;
    std::array<std::uint32_t, 32> post_constants(std::uint32_t width, std::uint32_t height, float mode, float x, float y) const;
    void set_common(ID3D12GraphicsCommandList* list, const std::array<std::uint32_t, 32>& constants);
    ComPtr<ID3D12RootSignature> root_;
    ComPtr<ID3D12CommandSignature> command_;
    ComPtr<ID3D12PipelineState> init_cull_, cull_;
    ComPtr<ID3D12PipelineState> depth_, scene_, extract_, blur_, tone_;
};
struct SceneProgram {
    framegraph::GraphDescription graph;
    std::vector<PassCallback> callbacks;
    framegraph::ResourceId backbuffer, readback, cull_readback;
};
SceneProgram make_scene_program(SceneRenderer& renderer, SceneState& state, std::uint32_t width, std::uint32_t height, std::uint64_t readback_bytes, std::uint32_t seed);
}
