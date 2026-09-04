# Tier 3 reliability contract

Tier 3 adds no rendering feature. It strengthens only failure, lifetime, stress, package and audit evidence after PACT-70/Tier 2 are green.

- Repeat resize 960x540 → 640x360 → requested size plus minimize/restore five times in one bounded 100-frame run. Each true dimension change must drain fences, rebuild once and preserve GPU/CPU culling counts, final capture quality and Debug 0/0/0 on WARP and hardware.
- Keep shader missing/file/entry/compiler diagnostics, impossible adapter, watchdog, invalid graph, undeclared access and zero capture deadline fail-closed tests.
- Add a real-device descriptor-heap boundary test: last owned CPU/GPU indices succeed; capacity and non-visible GPU access throw typed `DescriptorExhausted` before any device command.
- Review fence wrap/lifetime from source: each frame context waits its own prior fence before allocator/arena reuse; resize/destruction drain outstanding contexts; `UINT64_MAX` is rejected before increment; waits and automatic runs are bounded. Do not attempt an artificial GPU hang.
- Preserve the existing real PACT-40 device-removal report as fault-path evidence and review current `GetCompletedValue == UINT64_MAX`, `GetDeviceRemovedReason` and DRED capture paths. Do not claim a newly induced removal.
- Final package must be rebuilt from a clean exact HEAD and pass two independent fresh-directory WARP extraction smokes. Large binaries stay ignored/local.
- Finish with the existing permitted read-only whole-diff auditor, then update the Draft PR. Never merge, tag, release or upload binaries.
