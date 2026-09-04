# Root cause packet: executor device removal / access violation

Observable symptom: first PACT-40 WARP validation exited with access violation 0xC0000005 during command submission and produced no report. WIC negotiation and performance warnings had been fixed first; repeating without PNG still crashed, excluding WIC/file output.

Minimal reproduction: 320x180, 8 frames, WARP, aliasing on, plan/raw/report outputs. A project-local diagnostic trace narrowed failure to after command recording and before successful submission.

Debug Layer evidence: a new pre-submit queue gate prevented execution and reported message ID 340: CreateUnorderedAccessView was called for a Resource without ALLOW_UNORDERED_ACCESS. It then reported ID 232 and DXGI_ERROR_INVALID_CALL device removal. The failed report preserved plan/pass, HRESULT and DRED capability.

Confirmed facts: ProbeA/B/ObservedA legitimately used UAV-capable non-RT textures. The application had reused that logical descriptor for the imported swapchain resource; the actual backbuffer has ALLOW_RENDER_TARGET, not ALLOW_UNORDERED_ACCESS. Arena view creation therefore emitted an invalid UAV for the import. Alias barriers, slot offsets, WIC and timestamp/readback buffers were not the owner.

Fix: give Backbuffer its own RT-capable, non-UAV logical descriptor matching the actual swapchain resource. Keep queue checks after executor setup and before every submission so descriptor/recording diagnostics fail before reaching GPU execution. WARP and hardware alias-on/off runs then completed with exact RGBA parity and 0/0/0 Debug Layer messages.

No unknown process was stopped and no GPU fault was intentionally injected.
