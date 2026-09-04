# Verification matrix

| Gate | Required evidence | Current state |
|---|---|---|
| Baseline | MSVC + portable core build/CTest | Passed: MQB/MSVC 19.51 + Clang 23, CTest 1/1 each |
| Core semantics | unit cases, RED/GREEN logs, typed errors | 41 unit cases passed on MSVC/Clang; preliminary review fixed |
| Property | 10,000 valid / 10,000 invalid, independent invariants | Pending |
| Sanitizers | ASan and UBSan execution | Pending |
| Determinism | repeated byte-identical canonical plans | Passed in unit and 256-case property smoke; execution plan extension pending |
| Runtime | hardware interactive, WARP headless, adapter report | Pending |
| Barriers/alias | real heap/resource counts, on/off byte parity | Pending |
| Debug | error 0 / corruption 0 / unclassified warning 0 | Pending |
| Stress | 1,000 WARP + hardware frames; resize; minimize/restore | Pending |
| Negative | shader, unsupported adapter, invalid graph, timeout | Pending |
| Capture | seed/frame/camera, dimensions/color/hash, PNG | Pending |
| Inspector | actual browser desktop/narrow, selection/highlight, console | Pending |
| Package | fresh unzip 240-frame WARP smoke, JSON/PNG/alias/debug | Pending |
| CI | Windows and Ubuntu core/unit/property/canonical/build/links | Pending |
| Audit | fresh read-only full origin/main...HEAD review | Pending |
| Delivery | clean exact HEAD, ZIP + SHA manifest, push, Draft PR | Pending |
