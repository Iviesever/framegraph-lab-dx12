# Native D3D12 backend

The runtime foundation is native Win32/DXGI/D3D12. `Dx12Context` enables the Debug Layer before creating a device and attempts to enable per-process DRED breadcrumbs/page-fault diagnostics. Explicit hardware and WARP selection are distinct; automatic mode tries hardware in high-performance preference order and falls back to WARP. Adapter/vendor/device/driver/feature level are observed, not inferred from a marketing name.

Three frame contexts own independent command allocators, command lists and fence values. Recording waits only for the context being reused. Submission executes, presents, then signals its fence; a failed Present is checked after a signal attempt so queued work has a tracked completion. Resize and explicit shutdown wait the outstanding context fences. All waits are bounded, automatic runs also have an overall watchdog, and no detached threads exist.

`Win32Window` owns its HWND and registered class. Headless mode uses a hidden native window/swapchain. Resize releases backbuffers/RTV heap after fence completion, calls ResizeBuffers and recreates views. Zero client dimensions suspend recording. Native input events record camera, pause, step, reset, alias and resource-view requests for the scene layer. The foundation's stress switch exercises three sizes and minimize/restore.

COM interfaces use ComPtr; event HANDLE, window, and class ownership are RAII. Failure reports retain HRESULT, device-removed reason, graph/pass labels and available DRED data. DRED capability is present but no deliberate GPU removal/hang has been induced. Shader compilation preserves filename, entry, target and compiler diagnostics, including missing files and entries.

Debug queue collection counts every warning/error/corruption without an allow-list. Unknown warnings fail the run. Informational messages do not fail it. A bounded collection limit fails explicitly instead of silently dropping messages.

PACT-30 verified clear/present on an NVIDIA RTX 4070 Laptop GPU and WARP, plus visible resize/minimize/restore. PACT-40 replaced the temporary commands with `Dx12GraphExecutor`: each frame has an actual placed-resource arena, descriptor heaps, timestamp queries and pixel readback. The executor records only the ordered barriers in `CompiledPlan`; native state conversion is an exhaustive enum map. Callback access goes through `Dx12PassContext`, which rejects undeclared resources/usages before submission.

The executor validation graph proves real Texture UAV work, a real UAV ordering barrier, actual A→B memory reuse, named alias barriers, reference mode and exact on/off readback parity. Initial activation, RT/DS initialization and Common epilogues come from the plan. The current probe is intentionally a solid procedural validation image; it does not satisfy the final scene. PACT-50 replaces it with the HDR/depth/bloom/tone-map callbacks on the same executor.
