# Verification matrix

| Gate | Required evidence | Current state |
|---|---|---|
| Baseline | MSVC + portable core build/CTest | Passed: MQB/MSVC 19.51 + Clang 23, CTest 1/1 each |
| Core semantics | unit cases, RED/GREEN logs, typed errors | 41 compiler + 30 planner/oracle cases passed; IndirectArgument RED/GREEN included |
| Property | 10,000 valid / 10,000 invalid, independent invariants | Passed final MSVC Release and Clang ASan+UBSan |
| Sanitizers | ASan and UBSan execution | Passed CTest 11/11 and 10k + 10k sweep after PACT-70 Core change |
| Determinism | repeated byte-identical canonical plans | Passed full-plan sweeps, fresh processes, parsed JSON and MSVC/Clang equality |
| Runtime | hardware interactive, WARP headless, adapter report | Full hardware/WARP scene, PACT-70 CPU/GPU validation and 120-frame hardware passed |
| Barriers/alias | real heap/resource counts, on/off byte parity | WARP+hardware probe passed; actual 589824 vs 851968 bytes, exact RGBA parity |
| GPU culling | compute visible IDs, indirect args/draw, CPU oracle and parity | WARP+hardware fixed-frame 149/160 count, CPU/GPU exact RGBA, stable same-source identities, Debug 0/0/0 |
| Debug | error 0 / corruption 0 / unclassified warning 0 | 0/0/0 in hardware/WARP PACT-70 and full scene; final stress pending |
| Stress | 1,000 WARP + hardware frames; resize; minimize/restore | P0 passed on 7f90d9f; PACT-70 final-head recertification pending |
| Negative | shader, unsupported adapter, invalid graph, timeout | Shader, adapter, watchdog, CLI, capture deadline, invalid graph and undeclared access passed |
| Capture | seed/frame/camera, dimensions/color/hash, PNG | Hardware/WARP scene PNG/distribution/hash and same-adapter parity passed |
| Inspector | actual browser desktop/narrow, selection/highlight, console | Passed: console 0, linked selections, 375px page overflow 0 |
| Package | fresh unzip 240-frame WARP smoke, JSON/PNG/alias/debug | P0 passed on 7f90d9f; PACT-70 final-head recertification pending |
| CI | Windows and Ubuntu core/unit/property/canonical/build/links | P0 run 33878631431 green; PACT-70 push/CI pending |
| Audit | fresh read-only full origin/main...HEAD review | P0 whole-diff and ABI fix approved; PACT-70 final whole-diff audit pending |
| Delivery | clean exact HEAD, ZIP + SHA manifest, push, Draft PR | P0 baseline complete; PACT-70 package/PR update pending |
