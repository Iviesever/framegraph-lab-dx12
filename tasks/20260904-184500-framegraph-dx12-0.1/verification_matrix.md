# Verification matrix

| Gate | Required evidence | Current state |
|---|---|---|
| Baseline | MSVC + portable core build/CTest | Passed: MQB/MSVC 19.51 + Clang 23, CTest 1/1 each |
| Core semantics | unit cases, RED/GREEN logs, typed errors | 41 compiler + 29 planner/oracle cases passed; review fixes green |
| Property | 10,000 valid / 10,000 invalid, independent invariants | Passed final MSVC Release and Clang ASan+UBSan |
| Sanitizers | ASan and UBSan execution | Passed CTest 6/6 and 10k + 10k sweep |
| Determinism | repeated byte-identical canonical plans | Passed full-plan sweeps, fresh processes, parsed JSON and MSVC/Clang equality |
| Runtime | hardware interactive, WARP headless, adapter report | Full hardware/WARP scene and visible 120-frame hardware passed |
| Barriers/alias | real heap/resource counts, on/off byte parity | WARP+hardware probe passed; actual 589824 vs 851968 bytes, exact RGBA parity |
| Debug | error 0 / corruption 0 / unclassified warning 0 | 0/0/0 in hardware/WARP full scene; final stress pending |
| Stress | 1,000 WARP + hardware frames; resize; minimize/restore | Passed on cb592de; final-head recertification pending |
| Negative | shader, unsupported adapter, invalid graph, timeout | Shader, adapter, watchdog, CLI, capture deadline, invalid graph and undeclared access passed |
| Capture | seed/frame/camera, dimensions/color/hash, PNG | Hardware/WARP scene PNG/distribution/hash and same-adapter parity passed |
| Inspector | actual browser desktop/narrow, selection/highlight, console | Passed: console 0, linked selections, 375px page overflow 0 |
| Package | fresh unzip 240-frame WARP smoke, JSON/PNG/alias/debug | Passed cb592de; final-head recertification pending |
| CI | Windows and Ubuntu core/unit/property/canonical/build/links | Windows green; Ubuntu expected compatibility fix pending rerun |
| Audit | fresh read-only full origin/main...HEAD review | Complete: one CI Blocker fixed, progress Medium; fix review pending |
| Delivery | clean exact HEAD, ZIP + SHA manifest, push, Draft PR | Pending |
