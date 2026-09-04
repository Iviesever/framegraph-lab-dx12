# Tier 3 fence and device-removal review

Reviewed `context.hpp/.cpp`, `executor.cpp`, `diagnostics.hpp/.cpp`, the PACT-40 removal root-cause packet and current negative/stress results.

## Fence lifetime

- The timeline begins at 1. `submit_frame` rejects `next_fence_ == UINT64_MAX` before executing or incrementing, so `UINT64_MAX - 1` is the last assignable value and unsigned wrap cannot occur.
- `begin_frame` derives one of three frame-context indices and waits that context's prior fence before resetting its allocator/list. `Dx12GraphExecutor::record` uses the same index; its arena, timestamp/query buffers, culling readback and pixel readback cannot be overwritten while that context is in flight.
- `submitted` marks only the just-signaled frame pending. A later reuse occurs after `begin_frame` has waited, then `collect` maps its timestamps. Final capture waits the selected frame fence and drains outstanding contexts before mapping all results.
- Resize and alias-policy rebuilds explicitly wait idle before destroying executors/resources. Executor and context destructors also use bounded waits as fallback. No detached worker owns frame resources.
- Every wait uses the configured finite timeout; automatic runs also use a finite watchdog. The 100-frame repeated WARP/hardware test completed 15 resize drains, 5 minimize/restore cycles and 16 graph compilations with Debug 0/0/0.

No fence code change was justified by this review.

## Device removal and diagnostics

- `wait` treats `ID3D12Fence::GetCompletedValue() == UINT64_MAX` as device removal and records `GetDeviceRemovedReason` plus DRED output.
- Every native HRESULT passes through `check_hr`. When `GetDeviceRemovedReason` is failed, it throws typed `DeviceRemoved` with original HRESULT, removal reason and `removal_diagnostics`.
- `removal_diagnostics` queries `ID3D12DeviceRemovedExtendedData1` and records the auto-breadcrumb command-list name, completed breadcrumb operation count and page-fault virtual address when available. It does not traverse queue or allocation-name nodes. Missing DRED support is stated explicitly.
- Runtime failure handling retains the current compiled graph identity/current pass, HRESULT, removal reason, diagnostic text and collected Debug messages in the report.
- PACT-40's real invalid imported-UAV attempt reached device removal and produced the recorded `root-cause-executor-device-remove.md`; the queue pre-submit gate and descriptor fix then passed WARP/hardware. Tier 3 does not intentionally hang or remove a healthy device, so it makes no new induced-removal claim.

The current paths are complete for observed removal. Remaining risk is hardware/driver-specific removal behavior that cannot be safely forced in this delivery.
