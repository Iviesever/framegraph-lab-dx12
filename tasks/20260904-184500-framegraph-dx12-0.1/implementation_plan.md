# Implementation blueprint

Use inline PACT execution with at most two read-only review agents. The goal explicitly authorizes this design and continuous implementation.

Incremental constraint accepted after PACT-20: before the next graphics step, add real mqb.json and a single source/build-policy inventory consumed or checked by both MQB and CMake. Local C++ build/run uses MQB; CMake handles portable/CI/install/export evidence. Then continue PACT-30/40/50/60/80 in order. Never repeat completed 00/10/20. Source-only future release policy and updated freeze/reset/Tier gates are in product_contract.md and repository rules.

| PACT | Files / responsibility | Acceptance gate |
|---|---|---|
| 00 | Root CMake/presets/license/rules/task contract; tests/unit/smoke.cpp | MQB MSVC and portable compiler smoke, CTest; baseline main pushed, fresh feature branch |
| 10 | include/framegraph/graph.hpp; src/core/graph.cpp, compiler.cpp, serialization.cpp; tests/unit/compiler_tests.cpp | RED/GREEN semantic cases; canonical repeat; unit/property smoke |
| 20 | include/framegraph/plan.hpp; src/core/allocation.cpp, barriers.cpp; tests/unit/planner_tests.cpp; tests/property/graph_sweep.cpp | 10k valid + 10k invalid, independent intervals/barrier/alias oracle and stable identity |
| 30 | src/d3d12/context.*, diagnostics.*, capture.*; src/app/main.cpp, options.* | clear/present hardware; WARP headless; shader/adapter/timeout negatives; bounded waits |
| 40 | src/d3d12/executor.*, arena.*; tests/d3d12/validate.ps1 | actual placed heaps, plan barriers, access guard, real alias on/off RGBA parity |
| 50 | src/app/scene.*, shaders/scene.hlsl, post.hlsl | seven real major passes; timestamp, deterministic neon scene, controls and capture metrics |
| 60 | tools/build_inspector.py; viewer/; report output | real data, offline interactive HTML, desktop/narrow and browser console checks |
| 70 | conditional GPU culling | gated by all P0/time/budget; otherwise explicitly skipped |
| 80 | scripts/verify.ps1, package.ps1; .github/workflows; docs/* | full matrix, clean rebuild/HEAD packages, fresh unzip run, independent audit, Draft PR |

Every behavior begins with a named failing acceptance check, then a minimal implementation and focused regression. A PACT is not complete until its stated gate passes and is committed/pushed. See task.md for immediate executable steps; expand each subsequent contract before touching its production files. No GPU result exists at baseline.
