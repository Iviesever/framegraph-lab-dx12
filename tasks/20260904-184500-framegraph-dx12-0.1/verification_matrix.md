# Verification matrix

| Gate | Required evidence | Current state |
|---|---|---|
| Baseline | MSVC + portable core build/CTest | Passed: MQB/MSVC 19.51 + Clang 23, CTest 1/1 each |
| Core semantics | unit cases, RED/GREEN logs, typed errors | 41 compiler + 30 planner/oracle cases passed; IndirectArgument RED/GREEN included |
| Property | 100,000 valid / 100,000 invalid, independent invariants | Passed final Tier 2 reserve code under MQB/MSVC Release |
| Sanitizers | ASan and UBSan execution | CTest 12/12 final code; initial reserve version also passed 100k + 100k sweep |
| Determinism | repeated byte-identical canonical plans | Passed full-plan sweeps, fresh processes, parsed JSON and MSVC/Clang equality |
| Runtime | hardware interactive, WARP headless, adapter report | Full hardware/WARP scene, PACT-70 CPU/GPU validation and repeated resize passed |
| Barriers/alias | real heap/resource counts, on/off byte parity | Passed: probe and default GPU scene compare complete on/off RGBA on WARP/hardware |
| GPU culling | compute visible IDs, indirect args/draw, CPU oracle and parity | WARP+hardware fixed-frame 149/160 count, CPU/GPU exact RGBA, stable same-source identities, Debug 0/0/0 |
| Performance | raw compile/allocation/barrier and native alias samples/medians | Clean 57dfbab: 31×1000, compile -24.65%, barrier -6.09%, stable identity/counts; 7 native pairs |
| Debug | error 0 / corruption 0 / unclassified warning 0 | Passed: 0/0/0 in final WARP/hardware parity, reliability and 1,000-frame runs |
| Stress | 100k+100k Core, 100k mutation, 1,000 WARP/hardware, resize | Passed in current-head verification summary |
| Negative | shader, adapter, graph/access/capture/descriptor bounds | All passed; descriptor test uses real WARP device and typed exhaustion |
| Capture | seed/frame/camera, dimensions/color/hash, PNG | Hardware/WARP scene PNG/distribution/hash and same-adapter parity passed |
| Inspector | actual browser desktop/narrow, selection/highlight, console | Passed: final 11-pass data, console 0, Indirect chain, 375px overflow 0 |
| Package | fresh unzip 240-frame WARP smoke, JSON/PNG/alias/debug | Passed twice; current `DELIVERY_MANIFEST.json` is authoritative for exact paths/hashes |
| CI | Windows and Ubuntu core/unit/property/canonical/build/links | Passed for current Draft PR head; GitHub checks are authoritative |
| Audit | fresh read-only full origin/main...HEAD review | No Blocker/High; all four Medium findings addressed and follow-up checked against current evidence |
| Delivery | clean exact HEAD, ZIP + SHA manifest, push, Draft PR | Complete; current-head summary/manifest, remote branch and Draft PR #1 are authoritative |
