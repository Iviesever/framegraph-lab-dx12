# Verification matrix

| Gate | Required evidence | Current state |
|---|---|---|
| Baseline | MSVC + portable core build/CTest | Passed: MQB/MSVC 19.51 + Clang 23, CTest 1/1 each |
| Core semantics | unit cases, RED/GREEN logs, typed errors | 41 compiler + 29 planner/oracle cases passed; review fixes green |
| Property | 10,000 valid / 10,000 invalid, independent invariants | Passed final MSVC Release and Clang ASan+UBSan |
| Sanitizers | ASan and UBSan execution | Passed CTest 6/6 and 10k + 10k sweep |
| Determinism | repeated byte-identical canonical plans | Passed full-plan sweeps, fresh processes, parsed JSON and MSVC/Clang equality |
| Runtime | hardware interactive, WARP headless, adapter report | PACT-30 clear/present passed; full scene remains pending |
| Barriers/alias | real heap/resource counts, on/off byte parity | WARP+hardware probe passed; actual 589824 vs 851968 bytes, exact RGBA parity |
| Debug | error 0 / corruption 0 / unclassified warning 0 | 0/0/0 in hardware/WARP executor probe; full scene pending |
| Stress | 1,000 WARP + hardware frames; resize; minimize/restore | Pending |
| Negative | shader, unsupported adapter, invalid graph, timeout | Shader file/entry/compiler, adapter, watchdog, CLI passed; capture/executor negatives pending |
| Capture | seed/frame/camera, dimensions/color/hash, PNG | Probe WIC/raw hash/parity passed; final visual scene pending |
| Inspector | actual browser desktop/narrow, selection/highlight, console | Passed: console 0, linked selections, 375px page overflow 0 |
| Package | fresh unzip 240-frame WARP smoke, JSON/PNG/alias/debug | Pending |
| CI | Windows and Ubuntu core/unit/property/canonical/build/links | Pending |
| Audit | fresh read-only full origin/main...HEAD review | Pending |
| Delivery | clean exact HEAD, ZIP + SHA manifest, push, Draft PR | Pending |
