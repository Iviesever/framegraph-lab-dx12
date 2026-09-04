# Persistent progress

Started 2026-09-04 18:45 UTC+8. Current PACT: 10 completed locally; next PACT: 20.

- Goal file read in full; original copied into task directory.
- Target directory was absent and created without overwriting files. No ancestor `.agents` folder was present at D:/ or D:/program; repository rules now exist.
- GitHub authenticated as requested owner; remote repository did not exist.
- Tool discovery: MQB 5.4.0, CMake, Ninja, VS 2026 and Windows SDK present. Portable compiler discovery ongoing.
- No active UE/AutomationTool/ShaderCompile/FrameGraph process observed during initial check.
- Exact HEAD: unborn main. No test, Debug Layer, capture or artifact result yet.
- PACT-00 local acceptance passed: MQB MSVC smoke exit 0; CMake MSVC Debug configure/build/CTest exit 0 (1/1); Clang 23 configure/build/CTest exit 0 (1/1). Logs are under evidence/pact00-*. No graphics result exists.
- Baseline commit is about to be created from this verified tree, then public repository creation and push. The exact baseline SHA will be recorded in the feature-branch checkpoint (a commit cannot truthfully embed its own hash).

## PACT-00 checkpoint

Public repository created: https://github.com/Iviesever/framegraph-lab-dx12 . Baseline main: `fb23f40818661ed791854181f33ee22bf6ee57fc`, pushed and fetched. Feature branch was created from that actual origin/main. Working directory remains the requested standalone checkout.

## PACT-10

Pure Core now contains strong IDs/descriptors, typed expected errors, bounds/usage validation, RAW/WAR/WAW, stable topology, closed cycle diagnostics, conservative root closure, inclusive lifetimes, canonical JSON and FNV-1a 64-bit non-cryptographic identity. Valid UTF-8 round-trips; malformed bytes are escaped. Exported resource contents remain live through graph completion.

RED evidence: 25/25 validation cases failed against a compile stub; 14 added schedule/serialization cases failed; two review regressions failed. GREEN: 41/41 unit cases, all fixed. Preliminary read-only compiler review found UTF-8 escaping and export lifetime boundary; both reproduced and fixed. Remaining edge-limit/import-state coverage is tracked for full verification.

Commands (all exit 0): MQB compiler tests; `scripts/with-msvc.ps1 cmake --build --preset msvc-debug`; `ctest --preset msvc-debug`; portable Clang build and `ctest --preset core-clang`. CTest 3/3 on each compiler, property smoke 256 valid + 256 invalid with independent pairwise dependencies, culling/topology/lifetime and identity oracle. See evidence/pact10-*. Tested source changes are in the PACT-10 commit containing this entry; exact post-commit receipt is written to ignored artifacts/checkpoints/pact10.json.

No D3D12, Debug Layer, capture, heap or runtime artifact result exists yet. Next contract: PACT-20 reusable slots, explicit state/alias/UAV plan, independent allocation/state oracle and 10k + 10k sweep.
