# Testing and evidence

The test pyramid has focused pure unit binaries, independent randomized oracles, native CPU shader tests, real hardware/WARP subprocess validators, static Inspector validation and clean-package execution. GPU commands run serially; no script launches two FrameGraphLab GPU processes together.

MQB primary commands:

```powershell
./scripts/build.ps1 compiler_tests -Configuration debug -Run
./scripts/build.ps1 planner_tests -Configuration debug -Run
./scripts/build.ps1 boundary_tests -Configuration debug -Run
./scripts/build.ps1 property -Configuration release -Run --cases 10000
./scripts/build.ps1 fuzz -Configuration debug -Run --iterations 10000
./scripts/build.ps1 app -Configuration release
```

CMake evidence:

```powershell
./scripts/with-msvc.ps1 cmake --preset msvc-debug
./scripts/with-msvc.ps1 cmake --build --preset msvc-debug
ctest --preset msvc-debug
cmake --preset core-clang && cmake --build --preset core-clang && ctest --preset core-clang
cmake --preset core-gcc && cmake --build --preset core-gcc && ctest --preset core-gcc
cmake --preset core-sanitized && cmake --build --preset core-sanitized && ctest --preset core-sanitized
```

Structured randomized verification executes 10,000 valid and 10,000 invalid graphs per `--cases 10000`. Its pairwise hazard and brute-force topo/lifetime checks do not call production helpers. The allocation oracle checks byte intersections at every active pass position and mutates plans to prove it rejects live overlap and transition-before-activation. State simulation checks transition continuity, UAV rules and final import states. Exact-bound tests cover 65,536 usages and 262,144 edges plus rejection beyond.

`fuzz_compile.cpp` exports `LLVMFuzzerTestOneInput`; local bounded mutation uses a fixed seed and limit. LLVM-MinGW rejects coverage-guided `-fsanitize=fuzzer`, so no local coverage-guided claim is made. ASan+UBSan runs full structured sweep, where successful planner paths receive more coverage than random invalid bytes.

Native validators:

- `runtime_smoke.py`: bounded hardware/WARP context/frames/Debug report.
- `runtime_negative.py`: adapter, watchdog and CLI failures.
- `validate_executor.py`: probe UAV/alias/placed heaps/PNG/plan/timing/parity.
- `executor_negative.py`: invalid graph, undeclared access, capture deadline.
- `validate_scene.py`: required passes/formats/pixels/timing and scene parity.
- `validate_inspector.py`: embedded-byte/provenance/offline contract; browser QA is separately recorded.

The final `scripts/verify.ps1` binds logs to a clean SHA, runs all local gates/stress, generates the primary artifacts and calls package/clean-extraction validation. `artifacts/checkpoints/` is the machine-readable source of exact post-commit results. Checked `tasks/.../evidence` contains RED/GREEN development logs; development reports explicitly say dirty/older SHA when appropriate.
