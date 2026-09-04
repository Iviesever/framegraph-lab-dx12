# Product contract — FrameGraphLab 0.1 candidate

## P0

Every requirement in goal-objective.md sections PACT-00/10/20/30/40/50/60/80 and its final completion checklist is mandatory. Progress must separate implemented, tested, and pending. All 42 final conditions must have evidence before declaring completion.

- Pure C++23 graph API, typed errors, validated descriptors/handles/usages, RAW/WAR/WAW, stable edges/topology, cycle chain, root-based culling, whole-resource lifetimes and states, canonical JSON and deterministic identity.
- Pure stable slot allocator: alignment/overflow/heap-class/compatibility/dedicated checks, reference mode, independently checked live intervals, alias chains, state continuity, UAV ordering and imported final states.
- 10,000 valid and 10,000 invalid graph sweep, unit/fuzz/determinism tests, MSVC Debug/Release, portable Clang/GCC, ASan/UBSan and clean rebuild.
- Native Win32/D3D12 runtime; hardware/WARP selection; Debug Layer; bounded waits; three frames in flight; safe resize/minimize/restore; diagnostics and RAII.
- The backend uses the same plan, actual heaps/placed resources and alias barriers, declares accesses, queries real allocation requirements, and exposes real GPU timestamps.
- Procedural 1280x720 scene: depth, HDR, bloom extract/H/V, tone map and present; camera, pause/step/reset, alias toggle and debug view.
- Deterministic capture at seed 24301; fixed logical frame/camera; WIC PNG; byte-identical alias-on/off RGBA on the same adapter; capture metrics and hashes.
- Self-contained interactive HTML from real plan/report/capture; browser desktop/narrow/selection/highlight/console verification.
- Full Windows/hardware/WARP/negative/stress/resize/parity validation, artifact manifests/SHA, fresh extracted ZIP smoke, CI and complete portfolio documentation.
- Read-only independent audit; clean source revision; pushed feature branch and Draft PR based on actual origin/main; no merge/tag/release.

## P1 / P2

P1 is PACT-70 GPU culling ONLY after all P0 and package gates are green, before 2026-09-05 09:30 JST, with >25% reset budget. P2 is explanatory polish and drills within P0 scope. No optional feature may weaken P0 or run after feature freeze.

## Boundaries

Whole resources, single direct queue, no subresource ranges, async compute, external assets, engines, ECS, external UI framework, production hot reload, network functionality, additional repositories or global services. Compile deterministic integer plans; runtime adapter/timing observations are provenance, not portable guarantees.

## Policies

Debug builds enable the Debug Layer before device creation and reject unavailable diagnostics explicitly. Errors/corruptions/unclassified warnings must be zero. No blanket filters. RT/DS alias activations are initialized by full clear. Capture only after a bounded fence wait and report write completion. Actual device heap bytes and predicted bytes are separate fields and are checked.

All work and artifacts remain inside this repository. Never kill unrelated processes. At most one owned GPU process. Local MSVC uses MQB where supported; required CMake presets remain independent portable entry points. No evidence currently establishes MQB as a UE build orchestrator, and UE is outside scope.

## Authorship

The user defined career goals, direction, scope, deadline, budget strategy and acceptance. Codex GPT-5.6 Sol supplies architecture detail, implementation, tests, debugging, capture, packaging, audit and docs. The user does not hand-write this delivery. Before interviews, the user must complete at least one live change drill and explain compile/lifetime/alias/barrier/fence/WARP.

## Time

Freeze: 2026-09-05 11:30 UTC+8. Internal stop: 15:00 UTC+8. External deadline: 15:30 UTC+8. At a pause/reset write a recoverable handoff; never mislabel incomplete P0 as delivered.
