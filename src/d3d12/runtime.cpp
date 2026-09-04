#include "runtime.hpp"
#include "revision.hpp"
#include "context.hpp"
#include <memory>
namespace fgl {
RuntimeReport run_clear_demo(const Options& options) {
    RuntimeReport report;
    report.git_sha = build_revision::sha; report.source_clean = build_revision::clean;
    std::unique_ptr<Dx12Context> context;
    try {
        context = std::make_unique<Dx12Context>(options, report);
        const auto start = GetTickCount64();
        while (!context->window().closed() && (!options.frames || report.frames < options.frames)) {
            context->window().pump();
            if (context->window().closed()) break;
            if (options.frames && GetTickCount64() - start > options.watchdog_ms) throw GpuFailure("WatchdogTimeout", "automatic run exceeded watchdog");
            if (options.resize_stress) {
                if (report.frames == 3) context->window().set_size(960, 540);
                if (report.frames == 6) context->window().set_size(640, 360);
                if (report.frames == 9) context->window().set_size(options.width, options.height);
                if (report.frames == 12) context->window().minimize_restore();
            }
            if (!context->sync_size()) { MsgWaitForMultipleObjects(0, nullptr, FALSE, 10, QS_ALLINPUT); continue; }
            auto& frame = context->begin_frame();
            D3D12_RESOURCE_BARRIER barrier{}; barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition = {context->backbuffer(), D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET};
            frame.list->ResourceBarrier(1, &barrier);
            const float clear[]{0.015f, 0.045f, 0.12f, 1.f};
            frame.list->ClearRenderTargetView(context->backbuffer_rtv(), clear, 0, nullptr);
            std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter); frame.list->ResourceBarrier(1, &barrier);
            context->submit_frame();
        }
        context->wait_idle(); context->collect_debug();
        if (report.debug_errors || report.debug_warnings || report.debug_corruptions) throw GpuFailure("DebugLayer", "unclassified D3D12 diagnostics fail validation");
        report.success = true;
    } catch (const GpuFailure& failure) {
        report.failure_code = failure.category; report.error = failure.what(); report.hresult = failure.code; report.device_removed_reason = failure.removed_reason;
        report.device_diagnostics = failure.diagnostics;
        if (context) { report.current_pass = context->current_pass; report.error += " pass=" + context->current_pass; try { context->collect_debug(); } catch (...) {} }
    } catch (const std::exception& failure) { report.failure_code = "RuntimeFailure"; report.error = failure.what(); }
    return report;
}
}
