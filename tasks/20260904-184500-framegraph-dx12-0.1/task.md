# FrameGraphLab implementation tasks

> Agent workers: use executing-tasks inline; use read-only review subagents only for bounded independent review.

Goal: deliver all P0 criteria in goal-objective.md with exact revision evidence.
Architecture: pure compiler/allocation/barrier plan, native D3D12 consumer, procedural scene and offline inspector.
Stack: C++23, CMake 3.28+, MSVC x64, Windows SDK/D3D12/DXGI/D3DCompiler/WIC, portable Clang/GCC.

## Immediate sequence

- [x] PACT-00: `mqb run tests/unit/smoke.cpp --std 23 --debug --no-discover`; expect exit 0 and smoke message. Configure/build/CTest MSVC Debug and portable core. Commit baseline main, create public repo, push, fetch and branch from origin/main.
- [x] PACT-10a: define GraphDescription API and a failing `invalid_handle` test using out-of-range ResourceId; `GraphCompiler::compile` returns typed error. Run real RED, then validation GREEN.
- [x] PACT-10b: add tests for descriptor/usage/initialization/duplicates/bounds, then implement typed validation.
- [x] PACT-10c: explicit RAW/WAR/WAW edge tests, stable topology and cycle chain tests; implement hazard scan and deterministic traversal.
- [x] PACT-10d: root closure/lifetime tests and canonical byte/hash repeat; implement culling, lifetime and serialization. Unit and property smoke; checkpoint.
- [x] PACT-20: independent allocation/transition/UAV/alias tests first; stable greedy slots, wrap activation and transient Common epilogue; 20k sweep; checkpoint.
- [x] PACT-30: native hardware/WARP device, frame/fence lifecycle, clear/present, resize/minimize/restore, diagnostics and shader errors.
- [x] PACT-40: Core-plan executor, actual per-frame placed arenas, automatic barriers, timestamps/readback/PNG and hardware/WARP on/off RGBA parity.
- [x] PACT-50: real Depth/HDR/Bloom/ToneMap scene, controls, GPU timing, deterministic WIC capture and hardware/WARP scene parity.
- [ ] PACT-60 through 80: follow blueprint order, write each concrete acceptance test before the respective production change and append commands/results to evidence. Update this checklist as contracts land.

No unrelated repository writes. Feature changes on `feat/framegraph-dx12-0.1` only. The user-requested new repository directory is the isolated workspace; no second checkout is needed.
