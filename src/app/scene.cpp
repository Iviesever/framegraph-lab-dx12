#include "scene.hpp"
#include <DirectXMath.h>
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
namespace fgl {
using namespace framegraph;
namespace {
D3D12_SHADER_BYTECODE code(ID3DBlob* blob) { return {blob ? blob->GetBufferPointer() : nullptr, blob ? blob->GetBufferSize() : 0}; }
D3D12_GRAPHICS_PIPELINE_STATE_DESC base_pipeline(ID3D12RootSignature* root, ID3DBlob* vs, ID3DBlob* ps) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
    desc.pRootSignature = root; desc.VS = code(vs); desc.PS = code(ps); desc.SampleMask = UINT_MAX;
    desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    desc.RasterizerState.DepthClipEnable = TRUE; desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; desc.SampleDesc.Count = 1;
    return desc;
}
void view(ID3D12GraphicsCommandList* list, std::uint32_t width, std::uint32_t height) {
    const D3D12_VIEWPORT viewport{0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1};
    const D3D12_RECT rect{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    list->RSSetViewports(1, &viewport); list->RSSetScissorRects(1, &rect); list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
}
SceneRenderer::SceneRenderer(ID3D12Device* device, const std::filesystem::path& shader_directory) {
    D3D12_DESCRIPTOR_RANGE ranges[2]{};
    for (UINT i = 0; i < 2; ++i) { ranges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; ranges[i].NumDescriptors = 1; ranges[i].BaseShaderRegister = i; ranges[i].OffsetInDescriptorsFromTableStart = 0; }
    D3D12_ROOT_PARAMETER parameters[3]{};
    parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS; parameters[0].Constants.Num32BitValues = 32; parameters[0].Constants.ShaderRegister = 0;
    for (UINT i = 1; i < 3; ++i) { parameters[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; parameters[i].DescriptorTable.NumDescriptorRanges = 1; parameters[i].DescriptorTable.pDescriptorRanges = &ranges[i - 1]; parameters[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; }
    D3D12_STATIC_SAMPLER_DESC sampler{}; sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP; sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; sampler.MaxLOD = D3D12_FLOAT32_MAX; sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    D3D12_ROOT_SIGNATURE_DESC signature{}; signature.NumParameters = 3; signature.pParameters = parameters;
    signature.NumStaticSamplers = 1; signature.pStaticSamplers = &sampler;
    signature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    ComPtr<ID3DBlob> bytes, errors;
    const auto signature_result = D3D12SerializeRootSignature(&signature, D3D_ROOT_SIGNATURE_VERSION_1, &bytes, &errors);
    if (FAILED(signature_result)) throw GpuFailure("RootSignature", errors ? std::string(static_cast<const char*>(errors->GetBufferPointer()), errors->GetBufferSize()) : "root signature serialization failed", signature_result);
    check_hr(device->CreateRootSignature(0, bytes->GetBufferPointer(), bytes->GetBufferSize(), IID_PPV_ARGS(&root_)), "CreateRootSignature", device);
    auto depth_vs = compile_shader(shader_directory / "scene.hlsl", "DepthVS", "vs_5_0");
    auto scene_vs = compile_shader(shader_directory / "scene.hlsl", "SceneVS", "vs_5_0");
    auto scene_ps = compile_shader(shader_directory / "scene.hlsl", "ScenePS", "ps_5_0");
    auto full_vs = compile_shader(shader_directory / "post.hlsl", "FullscreenVS", "vs_5_0");
    auto extract_ps = compile_shader(shader_directory / "post.hlsl", "BloomExtractPS", "ps_5_0");
    auto blur_ps = compile_shader(shader_directory / "post.hlsl", "BloomBlurPS", "ps_5_0");
    auto tone_ps = compile_shader(shader_directory / "post.hlsl", "ToneMapPS", "ps_5_0");
    auto desc = base_pipeline(root_.Get(), depth_vs.Get(), nullptr); desc.NumRenderTargets = 0; desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    desc.DepthStencilState.DepthEnable = TRUE; desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    check_hr(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&depth_)), "CreatePipelineState(Depth)", device);
    desc = base_pipeline(root_.Get(), scene_vs.Get(), scene_ps.Get()); desc.NumRenderTargets = 1; desc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.DSVFormat = DXGI_FORMAT_D32_FLOAT; desc.DepthStencilState.DepthEnable = TRUE; desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    check_hr(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&scene_)), "CreatePipelineState(Scene)", device);
    desc = base_pipeline(root_.Get(), full_vs.Get(), extract_ps.Get()); desc.NumRenderTargets = 1; desc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    check_hr(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&extract_)), "CreatePipelineState(BloomExtract)", device);
    desc.PS = code(blur_ps.Get()); check_hr(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&blur_)), "CreatePipelineState(BloomBlur)", device);
    desc.PS = code(tone_ps.Get()); desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    check_hr(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&tone_)), "CreatePipelineState(ToneMap)", device);
}
std::array<std::uint32_t, 32> SceneRenderer::constants(std::uint32_t width, std::uint32_t height, const SceneState& state) const {
    using namespace DirectX;
    std::array<std::uint32_t, 32> values{};
    const float angle = state.yaw + static_cast<float>(state.logical_frame) * .004f;
    const auto eye = XMVectorSet(std::sin(angle) * 28.f, 13.f, std::cos(angle) * 28.f, 1.f);
    const auto view_matrix = XMMatrixLookAtLH(eye, XMVectorSet(0, 3.5f, 0, 1), XMVectorSet(0, 1, 0, 0));
    const auto projection = XMMatrixPerspectiveFovLH(XM_PI / 3.f, static_cast<float>(width) / static_cast<float>(height), .1f, 120.f);
    XMFLOAT4X4 stored; XMStoreFloat4x4(&stored, XMMatrixTranspose(view_matrix * projection));
    std::memcpy(values.data(), &stored, sizeof(stored));
    values[16] = std::bit_cast<std::uint32_t>(static_cast<float>(state.logical_frame) / 60.f);
    values[17] = state.seed; values[18] = state.logical_frame; values[19] = std::bit_cast<std::uint32_t>(angle);
    values[20] = std::bit_cast<std::uint32_t>(static_cast<float>(width)); values[21] = std::bit_cast<std::uint32_t>(static_cast<float>(height));
    values[22] = std::bit_cast<std::uint32_t>(1.f / width); values[23] = std::bit_cast<std::uint32_t>(1.f / height);
    return values;
}
std::array<std::uint32_t, 32> SceneRenderer::post_constants(std::uint32_t width, std::uint32_t height, float mode, float x, float y) const {
    std::array<std::uint32_t, 32> values{};
    values[20] = std::bit_cast<std::uint32_t>(static_cast<float>(width)); values[21] = std::bit_cast<std::uint32_t>(static_cast<float>(height));
    values[22] = std::bit_cast<std::uint32_t>(1.f / width); values[23] = std::bit_cast<std::uint32_t>(1.f / height);
    values[24] = std::bit_cast<std::uint32_t>(mode); values[25] = std::bit_cast<std::uint32_t>(x); values[26] = std::bit_cast<std::uint32_t>(y); return values;
}
void SceneRenderer::set_common(ID3D12GraphicsCommandList* list, const std::array<std::uint32_t, 32>& values) {
    list->SetGraphicsRootSignature(root_.Get()); list->SetGraphicsRoot32BitConstants(0, 32, values.data(), 0);
}
void SceneRenderer::depth(Dx12PassContext& pass, ResourceId target, const SceneState& state) {
    auto* list = pass.list(); view(list, pass.width(), pass.height()); list->SetPipelineState(depth_.Get()); set_common(list, constants(pass.width(), pass.height(), state));
    const auto dsv = pass.dsv(target); list->OMSetRenderTargets(0, nullptr, FALSE, &dsv); list->DrawInstanced(6, 1, 0, 0); list->DrawInstanced(36, 160, 0, 1);
}
void SceneRenderer::scene(Dx12PassContext& pass, ResourceId target, ResourceId depth, const SceneState& state) {
    auto* list = pass.list(); view(list, pass.width(), pass.height()); list->SetPipelineState(scene_.Get()); auto values = constants(pass.width(), pass.height(), state); set_common(list, values);
    const auto rtv = pass.rtv(target); const auto dsv = pass.dsv(depth, true); list->OMSetRenderTargets(1, &rtv, FALSE, &dsv); list->DrawInstanced(6, 1, 0, 0); list->DrawInstanced(36, 160, 0, 1);
}
void SceneRenderer::extract(Dx12PassContext& pass, ResourceId source, ResourceId target) {
    auto* list = pass.list(); view(list, pass.width() / 2, pass.height() / 2); list->SetPipelineState(extract_.Get()); set_common(list, post_constants(pass.width() / 2, pass.height() / 2, 0, 0, 0));
    list->SetGraphicsRootDescriptorTable(1, pass.srv(source)); const auto rtv = pass.rtv(target); list->OMSetRenderTargets(1, &rtv, FALSE, nullptr); list->DrawInstanced(3, 1, 0, 0);
}
void SceneRenderer::blur(Dx12PassContext& pass, ResourceId source, ResourceId target, float x, float y) {
    auto* list = pass.list(); view(list, pass.width() / 2, pass.height() / 2); list->SetPipelineState(blur_.Get()); set_common(list, post_constants(pass.width() / 2, pass.height() / 2, 0, x, y));
    list->SetGraphicsRootDescriptorTable(1, pass.srv(source)); const auto rtv = pass.rtv(target); list->OMSetRenderTargets(1, &rtv, FALSE, nullptr); list->DrawInstanced(3, 1, 0, 0);
}
void SceneRenderer::tone(Dx12PassContext& pass, ResourceId hdr, ResourceId bloom, ResourceId target, const SceneState& state) {
    auto* list = pass.list(); view(list, pass.width(), pass.height()); list->SetPipelineState(tone_.Get()); set_common(list, post_constants(pass.width(), pass.height(), static_cast<float>(state.debug_view), 0, 0));
    list->SetGraphicsRootDescriptorTable(1, pass.srv(hdr)); list->SetGraphicsRootDescriptorTable(2, pass.srv(bloom)); const auto rtv = pass.rtv(target);
    list->OMSetRenderTargets(1, &rtv, FALSE, nullptr); list->DrawInstanced(3, 1, 0, 0);
}
SceneProgram make_scene_program(SceneRenderer& renderer, SceneState& state, std::uint32_t width, std::uint32_t height, std::uint64_t readback_bytes, std::uint32_t seed) {
    SceneProgram p; p.graph.scene_seed = seed;
    const auto half_width = std::max(1u, width / 2), half_height = std::max(1u, height / 2);
    p.graph.resources = {
        {"Depth", TextureDesc{width, height, Format::D32Float, false, true, false}},
        {"SceneHDR", TextureDesc{width, height, Format::Rgba16Float, true, false, false}},
        {"BloomA", TextureDesc{half_width, half_height, Format::Rgba16Float, true, false, false}},
        {"BloomB", TextureDesc{half_width, half_height, Format::Rgba16Float, true, false, false}},
        {"BloomC", TextureDesc{half_width, half_height, Format::Rgba16Float, true, false, false}},
        {"Backbuffer", TextureDesc{width, height, Format::Rgba8, true, false, false}, true, true, false, ResourceState::Present, ResourceState::Present},
        {"Readback", BufferDesc{readback_bytes}, true, false, false, ResourceState::CopyDest, ResourceState::CopyDest}
    };
    p.backbuffer = ResourceId{5}; p.readback = ResourceId{6};
    p.graph.passes = {
        {"DepthPrepass", {{ResourceId{0}, ResourceAccess::Write, Usage::DepthWrite}}},
        {"SceneHDR", {{ResourceId{0}, ResourceAccess::Read, Usage::DepthRead}, {ResourceId{1}, ResourceAccess::Write, Usage::RenderTarget}}},
        {"BloomExtract", {{ResourceId{1}, ResourceAccess::Read, Usage::ShaderRead}, {ResourceId{2}, ResourceAccess::Write, Usage::RenderTarget}}},
        {"BloomBlurHorizontal", {{ResourceId{2}, ResourceAccess::Read, Usage::ShaderRead}, {ResourceId{3}, ResourceAccess::Write, Usage::RenderTarget}}},
        {"BloomBlurVertical", {{ResourceId{3}, ResourceAccess::Read, Usage::ShaderRead}, {ResourceId{4}, ResourceAccess::Write, Usage::RenderTarget}}},
        {"ToneMap", {{ResourceId{1}, ResourceAccess::Read, Usage::ShaderRead}, {ResourceId{4}, ResourceAccess::Read, Usage::ShaderRead}, {p.backbuffer, ResourceAccess::Write, Usage::RenderTarget}}},
        {"Capture", {{p.backbuffer, ResourceAccess::Read, Usage::CopySource}, {p.readback, ResourceAccess::Write, Usage::CopyDest}}, true},
        {"Present", {{p.backbuffer, ResourceAccess::Read, Usage::Present}}}
    };
    p.graph.ordering.push_back({PassId{6}, PassId{7}});
    auto* r = &renderer; auto* s = &state;
    p.callbacks.resize(8);
    p.callbacks[0] = [r, s](Dx12PassContext& ctx) { r->depth(ctx, ResourceId{0}, *s); };
    p.callbacks[1] = [r, s](Dx12PassContext& ctx) { r->scene(ctx, ResourceId{1}, ResourceId{0}, *s); };
    p.callbacks[2] = [r](Dx12PassContext& ctx) { r->extract(ctx, ResourceId{1}, ResourceId{2}); };
    p.callbacks[3] = [r](Dx12PassContext& ctx) { r->blur(ctx, ResourceId{2}, ResourceId{3}, 1, 0); };
    p.callbacks[4] = [r](Dx12PassContext& ctx) { r->blur(ctx, ResourceId{3}, ResourceId{4}, 0, 1); };
    p.callbacks[5] = [r, s, backbuffer = p.backbuffer](Dx12PassContext& ctx) { r->tone(ctx, ResourceId{1}, ResourceId{4}, backbuffer, *s); };
    p.callbacks[6] = [backbuffer = p.backbuffer, readback = p.readback](Dx12PassContext& ctx) { ctx.readback(backbuffer, readback); };
    p.callbacks[7] = [backbuffer = p.backbuffer](Dx12PassContext& ctx) { (void)ctx.resource(backbuffer); };
    return p;
}
}
