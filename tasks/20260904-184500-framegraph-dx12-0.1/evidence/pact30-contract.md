# PACT-30 runtime acceptance before production code

Build a native runtime in src/d3d12/context.*, diagnostics.* and src/app/options.*, main.cpp. Start with a real executable whose runtime stub returns a typed failure, run tests expecting successful adapter/clear/present reporting (RED), then implement the native runtime. Complete a real hardware clear/present run followed by hidden-window WARP smoke before adding the graph executor or scene.

API contract:

- Options parse the required hardware/warp/headless/frames/scene-seed/capture/report/plan/aliasing switches with strict values, plus bounded fence and overall watchdog values. Hardware/warp conflict is a typed configuration failure. Executable-relative shader lookup is mandatory later.
- Context enables the Debug Layer before factory/device creation, selects hardware by performance preference or explicit WARP, and records adapter description/vendor/device/driver/feature level. Auto mode may fall back to WARP and reports it. Unsupported explicit adapter selection fails typed.
- A hidden HWND owns the headless swapchain; normal mode shows a 1280x720 Win32 client. Three frame contexts own separate command allocators/lists and fence values. Before reuse, wait only that context with a bounded event wait. No detached threads or per-frame full-queue flush.
- Resize drains outstanding contexts, releases backbuffers and descriptors, resizes and rebuilds, and later recompiles size-dependent graph. Zero-size/minimize suspends rendering; restore resumes. CLI stress mode can drive resize/minimize/restore without external UI automation.
- Every native handle/window/COM object is RAII. Device failures record HRESULT, removed reason and current graph/pass. Debug messages are collected without blanket filters; errors/corruption/unclassified warnings fail the validation run.
- Report success/failure and debug counts as real observations. Initial clear output is a temporary PACT-30 checkpoint and does not satisfy the scene/product goal.

P0 backend integration boundaries from Core review: actual default 64-KiB GetResourceAllocationInfo requirements, explicit activation even without memory reuse, full clear of newly activated RT/DS attachments, per-frame arenas protected by fences, and matching imported initial/final Present for reusable backbuffer plans.
