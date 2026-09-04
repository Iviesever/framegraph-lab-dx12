# Persistent progress

Started 2026-09-04 18:45 UTC+8. Current PACT: 20 completed locally; next PACT: 30.

- Goal file read in full; original copied into task directory.
- Target directory was absent and created without overwriting files. No ancestor `.agents` folder was present at D:/ or D:/program; repository rules now exist.
- GitHub authenticated as requested owner; remote repository did not exist.
- Tool discovery: MQB 5.4.0, CMake, Ninja, VS 2026 and Windows SDK present. Portable compiler discovery ongoing.
- No active UE/AutomationTool/ShaderCompile/FrameGraph process observed during initial check.
- At startup HEAD was unborn main and no test, Debug Layer, capture or artifact result existed.
- PACT-00 local acceptance passed: MQB MSVC smoke exit 0; CMake MSVC Debug configure/build/CTest exit 0 (1/1); Clang 23 configure/build/CTest exit 0 (1/1). Logs are under evidence/pact00-*. No graphics result exists.
- Baseline commit is about to be created from this verified tree, then public repository creation and push. The exact baseline SHA will be recorded in the feature-branch checkpoint (a commit cannot truthfully embed its own hash).

## PACT-00 checkpoint

Public repository created: https://github.com/Iviesever/framegraph-lab-dx12 . Baseline main: `fb23f40818661ed791854181f33ee22bf6ee57fc`, pushed and fetched. Feature branch was created from that actual origin/main. Working directory remains the requested standalone checkout.

## PACT-10

Pure Core now contains strong IDs/descriptors, typed expected errors, bounds/usage validation, RAW/WAR/WAW, stable topology, closed cycle diagnostics, conservative root closure, inclusive lifetimes, canonical JSON and FNV-1a 64-bit non-cryptographic identity. Valid UTF-8 round-trips; malformed bytes are escaped. Exported resource contents remain live through graph completion.

RED evidence: 25/25 validation cases failed against a compile stub; 14 added schedule/serialization cases failed; two review regressions failed. GREEN: 41/41 unit cases, all fixed. Preliminary read-only compiler review found UTF-8 escaping and export lifetime boundary; both reproduced and fixed. Remaining edge-limit/import-state coverage is tracked for full verification.

Commands (all exit 0): MQB compiler tests; `scripts/with-msvc.ps1 cmake --build --preset msvc-debug`; `ctest --preset msvc-debug`; portable Clang build and `ctest --preset core-clang`. CTest 3/3 on each compiler, property smoke 256 valid + 256 invalid with independent pairwise dependencies, culling/topology/lifetime and identity oracle. See evidence/pact10-*. Tested source changes are in the PACT-10 commit containing this entry; exact post-commit receipt is written to ignored artifacts/checkpoints/pact10.json.

PACT-10 pushed exact HEAD: `908430706625ff5f741a5875cb0f465323afd6a7`.

## PACT-20

Implemented `CompiledPlan` containing the same Core graph plus stable slot allocation and before/after/final barriers. Actual device requirements remain a backend responsibility. Alignment/overflow/class/compatibility/dedicated checks, reference mode, alias predecessor chains, positive savings/padding overhead, transition/UAV ordering, all-placed activation and unused imported final states are covered.

Validation: 41 compiler + 29 planner/oracle mutation unit cases. CTest 6/6 on MSVC Debug, MSVC Release, Clang and Clang ASan+UBSan. Full 10,000 valid and 10,000 invalid graph sweeps passed on final Core under MSVC Release and ASan+UBSan. Independent oracles check each active byte interval, activation before access/transition, alias chains, state continuity, UAV ordering, topology, lifetime and deterministic identity. Bounded mutation fuzz 2,000 inputs passed on each profile. Native MinGW libFuzzer mode is unsupported by the driver; no local coverage-guided result is claimed (see fuzz-notes).

Canonical fixture parsed by Python, preserves UTF-8, repeats across fresh processes, matches an independently computed identity, and is byte-identical between MSVC and Clang (`4b6a2cf15d85a354`). Core contains no Windows/D3D types. Logs: evidence/pact20-*. All final verification commands exited 0; the optional libFuzzer build rejection is separately documented.

Read-only review found a validation-oracle activation-order gap, reproduced by a corrupted plan and repaired. No confirmed production High/Blocker. Explicit initial activation, import replay preconditions and native heap-alignment boundaries are documented. Post-commit exact revision/clean-status receipt is written to artifacts/checkpoints/pact20.json.

No D3D12 execution, Debug Layer run, capture, real heap or runtime artifact result exists yet. Hardware discovery shows Intel UHD and NVIDIA RTX 4070 Laptop GPU; D3D12SDKLayers.dll is present. Next: PACT-30 runtime contract in evidence/pact30-contract.md, real clear/present then hidden WARP smoke. All other P0 graphics/package/CI/docs/audit/PR gates remain pending.
