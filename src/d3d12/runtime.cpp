#include "runtime.hpp"
#include "revision.hpp"
#include "context.hpp"
#include "executor.hpp"
#include "capture.hpp"
#include "app/probe_graph.hpp"
#include "app/scene.hpp"
#include "shader.hpp"
#include "framegraph/graph.hpp"
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
namespace fgl {
RuntimeReport run_clear_demo(const Options& options) {
    RuntimeReport report;
    report.git_sha = build_revision::sha; report.source_clean = build_revision::clean;
    std::unique_ptr<Dx12Context> context;
    std::unique_ptr<Dx12GraphExecutor> executor;
    std::unique_ptr<SceneRenderer> renderer;
    SceneState scene_state; scene_state.seed = options.scene_seed;
    bool aliasing = options.aliasing;
    try {
        context = std::make_unique<Dx12Context>(options, report);
        if (options.scene == SceneMode::NeonRuins) {
            const auto shader_directory = options.shader_directory.empty() ? executable_directory() / "shaders" : options.shader_directory;
            renderer = std::make_unique<SceneRenderer>(context->device(), shader_directory);
        }
        auto create_executor = [&] {
            const auto layout = readback_layout(context->device(), context->width(), context->height());
            auto install = [&](auto program) {
                if (options.validation_invalid_graph) program.graph.passes[0].usages[0].resource = framegraph::ResourceId{99999};
                if (options.validation_undeclared) {
                    const auto forbidden = program.readback;
                    program.callbacks[0] = [forbidden](Dx12PassContext& pass) { (void)pass.resource(forbidden); };
                }
                executor = std::make_unique<Dx12GraphExecutor>(*context, std::move(program.graph), std::move(program.callbacks), program.backbuffer, program.readback, report, aliasing);
            };
            if (options.scene == SceneMode::ExecutorProbe) install(make_probe_program(context->width(), context->height(), layout.bytes, options.scene_seed));
            else install(make_scene_program(*renderer, scene_state, context->width(), context->height(), layout.bytes, options.scene_seed));
            context->collect_debug();
            if (report.debug_errors || report.debug_warnings || report.debug_corruptions)
                throw GpuFailure("DebugLayer", "D3D12 diagnostics rejected during executor setup");
            if (options.barrier_trace) std::cerr << "barrier-plan " << framegraph::canonical_json(executor->plan()) << '\n';
            if (options.lifetime_trace) {
                for (std::size_t i = 0; i < executor->plan().graph.description.resources.size(); ++i) {
                    std::cerr << "resource[" << i << "] " << executor->plan().graph.description.resources[i].name << " lifetime=";
                    const auto& life = executor->plan().graph.lifetimes[i];
                    if (life) std::cerr << life->first << ".." << life->last; else std::cerr << "none";
                    const auto& allocation = executor->plan().allocation.resources[i];
                    if (allocation) std::cerr << " heap=" << allocation->heap << " offset=" << allocation->offset;
                    std::cerr << '\n';
                }
            }
            if (!options.plan.empty()) {
                if (options.plan.has_parent_path()) std::filesystem::create_directories(options.plan.parent_path());
                auto json = framegraph::canonical_json(executor->plan()); json.pop_back();
                json += ",\"git_sha\":" + framegraph::json_quote(report.git_sha) + ",\"source_clean\":" + std::string(report.source_clean ? "true" : "false") + "}";
                std::ofstream file(options.plan, std::ios::binary); file << json << '\n'; file.close();
                if (!file) throw GpuFailure("PlanOutput", "failed to write canonical plan");
            }
        };
        create_executor();
        const auto start = GetTickCount64();
        while (!context->window().closed() && (!options.frames || report.frames < options.frames)) {
            context->window().pump();
            if (context->window().closed()) break;
            const auto input = context->window().take_input();
            scene_state.yaw += input.yaw_delta;
            if (input.pause) scene_state.paused = !scene_state.paused;
            if (input.step) scene_state.single_step = true;
            if (input.reset) { scene_state.logical_frame = 0; scene_state.yaw = .7f; }
            if (input.debug_next) scene_state.debug_view = (scene_state.debug_view + 1) % 3;
            if (input.alias_toggle) { context->wait_idle(); executor.reset(); aliasing = !aliasing; create_executor(); }
            if (options.frames && GetTickCount64() - start > options.watchdog_ms) throw GpuFailure("WatchdogTimeout", "automatic run exceeded watchdog");
            if (options.resize_stress) {
                if (report.frames == 3) context->window().set_size(960, 540);
                if (report.frames == 6) context->window().set_size(640, 360);
                if (report.frames == 9) context->window().set_size(options.width, options.height);
                if (report.frames == 12) context->window().minimize_restore();
            }
            if (!context->window().width() || !context->window().height()) { MsgWaitForMultipleObjects(0, nullptr, FALSE, 10, QS_ALLINPUT); continue; }
            if (context->window().resized() && (context->width() != context->window().width() || context->height() != context->window().height())) {
                context->wait_idle(); executor.reset(); context->sync_size(); create_executor();
            } else context->sync_size();
            context->begin_frame(); executor->record(scene_state.logical_frame);
            context->collect_debug();
            if (report.debug_errors || report.debug_warnings || report.debug_corruptions)
                throw GpuFailure("DebugLayer", "D3D12 diagnostics rejected before submission");
            context->submit_frame(); executor->submitted();
            if (!scene_state.paused || scene_state.single_step) { ++scene_state.logical_frame; scene_state.single_step = false; }
            const wchar_t* debug_names[]{L"Final", L"HDR", L"Bloom"};
            context->window().title(L"FrameGraphLab | Frame " + std::to_wstring(scene_state.logical_frame) + L" | Alias " + (aliasing ? L"ON" : L"OFF") + L" | View " + debug_names[scene_state.debug_view] + L" | Space Pause · N Step · R Reset · A Alias · V View");
        }
        if (options.frames && report.frames != options.frames) throw GpuFailure("Interrupted", "automatic run closed before requested frame count");
        auto rgba = executor->finish(options.capture_timeout_ms);
        std::uint64_t non_black = 0;
        std::uint32_t low = 255, high = 0;
        std::array<bool, 4096> buckets{};
        for (std::size_t i = 0; i < rgba.size(); i += 4) {
            if (rgba[i] > 3 || rgba[i + 1] > 3 || rgba[i + 2] > 3) ++non_black;
            const auto luminance = (static_cast<std::uint32_t>(rgba[i]) * 54 + static_cast<std::uint32_t>(rgba[i + 1]) * 183 + static_cast<std::uint32_t>(rgba[i + 2]) * 19) >> 8;
            low = std::min(low, luminance); high = std::max(high, luminance);
            buckets[(rgba[i] >> 4) * 256 + (rgba[i + 1] >> 4) * 16 + (rgba[i + 2] >> 4)] = true;
        }
        report.non_black_fraction = static_cast<double>(non_black) / static_cast<double>(rgba.size() / 4);
        report.luminance_min = low; report.luminance_max = high;
        report.color_buckets = static_cast<std::uint32_t>(std::count(buckets.begin(), buckets.end(), true));
        report.pixel_hash = framegraph::stable_hash(std::string(reinterpret_cast<const char*>(rgba.data()), rgba.size()));
        if (!options.rgba.empty()) {
            if (options.rgba.has_parent_path()) std::filesystem::create_directories(options.rgba.parent_path());
            std::ofstream file(options.rgba, std::ios::binary); file.write(reinterpret_cast<const char*>(rgba.data()), static_cast<std::streamsize>(rgba.size())); file.close();
            if (!file) throw GpuFailure("CaptureOutput", "failed to write raw RGBA");
        }
        if (!options.capture.empty()) write_png(options.capture, context->width(), context->height(), rgba);
        context->collect_debug();
        if (report.debug_errors || report.debug_warnings || report.debug_corruptions) throw GpuFailure("DebugLayer", "unclassified D3D12 diagnostics fail validation");
        report.success = true;
    } catch (const GpuFailure& failure) {
        report.failure_code = failure.category; report.error = failure.what(); report.hresult = failure.code; report.device_removed_reason = failure.removed_reason;
        report.device_diagnostics = failure.diagnostics;
        if (context) { report.current_pass = context->current_pass; report.error += " pass=" + context->current_pass; try { context->wait_idle(); context->collect_debug(); } catch (...) {} }
    } catch (const std::exception& failure) { report.failure_code = "RuntimeFailure"; report.error = failure.what(); }
    return report;
}
}
