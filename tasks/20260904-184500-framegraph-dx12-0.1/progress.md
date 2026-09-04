# Persistent progress

Started 2026-09-04 18:45 UTC+8. Current PACT: 80 audit fixes; final recertification/CI/PR next.

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

## Incremental user constraint checkpoint

Reread AGENTS/rules/contract/blueprint/progress/handoff and fresh-fetched. Local and remote feature HEAD both `38fe422b632ef79074d892e202e6b87046018408`; ahead/behind 0/0; tree was clean. PACT-20 is already complete and pushed. No earlier PACT is restarted. Next atomic step strengthens MQB configuration and shared source/policy verification, then proceeds directly into D3D12 PACT-30. Updated release/freeze/reset/Tier policies are accepted in the contract and rules; no new repository is authorized.

Build supplement implemented: `build-manifest.json` is the sole source/policy inventory; generated strict `mqb.json` provides 7 existing targets/profiles and scripts/build.ps1 dispatches actual MQB compilation/link/archives. CMake consumes the same manifest and rejects target/global flag drift. Python checks reject missing/unlisted sources, policy changes and duplicate keys. MQB default plan entry and repeat incremental no-op were verified. Both tools resolve installed MSVC 14.51.36231. Raw compiler-PDB `/Zi` is explicitly unsupported by MQB 5.4, so both routes use `/Z7` instead. Static CRT policies now align MTd/MT.

Build supplement verification: fresh MQB Core/static + compiler/planner/property/plan commands passed; CMake MSVC and Clang CTest 8/8 passed; installed Clang Core exported package compiled and ran a separate consumer using transitive C++23. No source behavior changed except making an already-empty optional initializer explicit to remove Clang aggregate warnings. All logs are evidence/build-contract-*. Supplement checkpoint precedes PACT-30 and does not repeat earlier PACT work.

Build supplement pushed SHA: `61057208d58d7f0d3b29b5641526700baca003fc`.

## PACT-30

Native Win32/DXGI/D3D12 device, explicit hardware/WARP/auto selection, hidden-window headless mode, 3 frame allocators/lists/fence values, bounded waits/watchdog, resize/RTV rebuilding and zero-size suspension are implemented. COM/event/window/class are RAII. Debug Layer is enabled before device creation in both configurations; DRED capability is enabled when available. Failure reports retain HRESULT, removed reason, current graph/pass and available diagnostics. Shader errors retain file/entry/target/compiler output.

RED/GREEN: 12 typed CLI cases and 4 shader cases failed before implementation, then all passed. Native acceptance failed against RuntimeNotReady stub, then passed on real hardware and WARP. Windows CTest now 10/10 (86 unit cases across compiler/planner/options/shader); portable Core/CLI Clang CTest 9/9. MQB Debug and Release native builds passed; Release WARP smoke passed.

Hardware: NVIDIA GeForce RTX 4070 Laptop GPU, DXGI driver 31.0.15.5161, feature level 12.2. WARP: Microsoft Basic Render Driver, 10.0.22621.2506, feature level 12.1. Both final Debug runs rendered/presented 24 frames at 640x360 with 3 frames in flight, Debug Layer Error/Warning/Corruption 0/0/0 and DRED enabled. Visible hardware 60-frame run at 1280x720 completed 3 resizes, 1 minimize and 1 restore, also 0/0/0. Typed unsupported-adapter, watchdog and invalid CLI negatives passed. No owned GPU process remained.

Evidence: tasks/.../evidence/pact30-*.log and ignored artifacts/reports/runtime-*.json. Development reports truthfully indicate source_clean=false and the prior HEAD; after commit a clean rebuild/smoke receipt is written under artifacts/checkpoints/pact30.json. No PNG, real transient heap reuse, pixel parity, HDR/Bloom, GPU timestamps or final artifact is claimed by this clear/present checkpoint. PACT-40 directly follows.

PACT-30 pushed/checkpoint SHA: `e5e7b36eed9023c90b0d60a9119b22db9c07a529`; its clean WARP receipt is artifacts/checkpoints/pact30.json.

## PACT-40

The temporary clear path is removed. `Dx12GraphExecutor` consumes one `CompiledPlan`, maps its enums, and records stored transition/UAV/alias/final lists without a runtime planner. Callback access is guarded against undeclared resources/usages. Device requirements come from GetResourceAllocationInfo; every frame arena creates real class-specific ID3D12Heap and CreatePlacedResource objects at Core offsets, with descriptor bounds. RT/DS activation gets required full initialization. Imports borrow current backbuffer and owned frame readback buffers.

Each of 3 arenas created 4 placed resources. The executor probe contains 8 executed passes, including texture UAV clears, actual UAV ordering, A→B same-offset reuse, texture copies, readback and Present. Alias-on uses 589,824 bytes/resource arena versus 851,968 reference bytes (262,144 saved, 30.77% of reference); all-frame totals are 1,769,472 versus 2,555,904. Actual ID3D12Heap descriptions equal Core plan. On uses 1 reuse event; off uses 0. Both use placed resources and first-use activation barriers.

WARP and NVIDIA hardware each passed serial 8-frame alias-on/off validation with byte-identical 320x180 RGBA (`8d98f55d5c518d25` on this machine/adapter pair), valid WIC PNG, parsed plan/report, plan identity agreement, 8 timestamped pass samples and Debug 0/0/0. No cross-adapter hash guarantee is made, despite the observed match. Release WARP repeated successfully. Hardware/WARP evidence directories are printed in evidence/pact40-*-final.log; generated binaries/captures remain ignored.

RED acceptance first failed because no placed-resource result existed. First native attempt revealed WIC format negotiation and exact performance warnings. After resolving those, a pre-submit gate exposed message ID 340: imported backbuffer was incorrectly described as UAV, followed by ID 232 device removal. The root-cause packet records the evidence and ownership fix. Final setup/pre-submit diagnostics prevent invalid commands reaching GPU.

Typed invalid graph, undeclared access and zero capture deadline all failed closed with Debug counts 0/0/0. Barrier/lifetime trace switches are available. Windows CTest 10/10 and 87 internal cases pass; build output has no code warning. MQB Debug/Release are green. No process remains. PACT-50 now replaces the solid probe with the required depth/HDR/bloom/tone-map procedural scene; the probe is evidence, not the visual delivery.

PACT-40 pushed/checkpoint SHA: `d32878ed2cf306dee67a3cd887d8389dd9f2c1fd`; clean parity receipt: artifacts/checkpoints/pact40.json.

## PACT-50

Neon Ruins is the default 1280x720 program. Eight real passes run through the shared executor: DepthPrepass, SceneHDR, BloomExtract, BloomBlurHorizontal, BloomBlurVertical, ToneMap, Capture and Present. HLSL generates a procedural grid/floor and 160 instanced pillars with moving light, emissive cyan/magenta HDR, separable bloom and fullscreen tone mapping. No art assets/frameworks are used. Depth is D32, HDR/bloom are R16G16B16A16_FLOAT and the swapchain is RGBA8.

Validation first failed against the solid executor probe. It next exposed a precise HLSL reserved-token compile diagnostic, fixed by renaming the variable. WARP scene parity then passed. Hardware found a reproducible cross-format named-alias parity failure; the root-cause packet records its 63,435 differing bytes. Core now explicitly plans conservative native null-before activation while preserving A→B diagnostic chains. Planner RED/GREEN passed and the scope enters schema identity.

Final 640x360: WARP alias-on/off parity passed with hash `db90d293f9ed4b49`, 440 color buckets, actual 3,407,872 versus 4,456,448 bytes (1,048,576 saved). Hardware parity passed twice with hash `11588379d8a28a6a`, 439 buckets, actual 3,473,408 versus 4,521,984 (1,048,576 saved). Cross-adapter hashes are explicitly not compared. All four final policy runs had Debug 0/0/0, 8/8 pass timestamps with 12 samples each, nonblack ratio >20%, luminance range >=50, valid PNG/plan/report and exact raw parity.

Visible 1280x720 hardware ran 120 frames: logical bytes 16,588,800; committed requirements 17,694,720; actual heap 13,762,560; saved 3,932,160 (22.22%); 2 reuse events; 18 transitions and 5 activation barriers per graph; Debug 0/0/0; color buckets 450. Local GPU timing means: Depth 0.0165 ms, SceneHDR 0.0211, Extract 0.0075, H blur 0.0090, V blur 0.0090, ToneMap 0.0218, Capture 0.3079, Present 0.0015. These are observations, not SLA.

Graph-aware visible/hidden hardware 60-frame resize test passed 3 sizes, minimize/restore, and exactly 4 plan compilations (initial + each dimension change). Controls implemented: arrows/mouse orbit; pause, single step, reset, alias toggle with fence drain/recompile; final/HDR/bloom views. Default executable-relative shader lookup and manifest-owned asset copying work under the MQB run boundary.

Clang ASan+UBSan CTest 9/9 plus updated 10,000 valid/10,000 invalid full-plan sweep passed. Windows CMake/MSVC 10/10 and MQB scene/option/planner builds passed. After commit, clean-head WARP primary artifacts and parity receipt are generated before PACT-60. Inspector/report packaging, long stress and final clean package remain pending.

PACT-50 pushed/checkpoint SHA: `007e8bf609b09e2f0e456a81e58bf1237e7f64b1`; clean primary/parity receipt: artifacts/checkpoints/pact50.json. Primary plan identity `14099d56b14abe68`; pixel hash `3cd8249e79e81191`.

## PACT-60

`tools/build_inspector.py` now generates a 715,508-byte UTF-8 self-contained sample HTML from the exact clean primary plan/report/PNG. Static validation decodes each base64 payload and compares semantic JSON/image bytes, checks provenance/identity, sections/interactions/responsive CSS and no external URLs. Embedded PNG SHA-256: `fd2c80f9ef13d5e7ec4be85d2f390ddfb80eca03ff8b7b728da725b67a2157d2`.

Actual browser desktop/narrow QA and interaction passed after one responsive RED/GREEN fix. Console Error/Warning 0. At 375px client/scroll width is 360/360 and horizontal scroll is 0; internal data tracks scroll independently. SceneHDR and BloomC selections updated pass/resource/barrier detail correctly. Browser viewport reset, tab closed and owned server stopped. A 1440x1100 actual browser screenshot was visually checked.

Checked-in viewer sample data and two documentation PNGs are real PACT-50 outputs with explicit SHA/identity provenance. Large full artifacts remain ignored. Final artifacts must still be regenerated from final PACT-80 HEAD. Remaining: complete docs/CI, full clean rebuild, 1000-frame WARP/hardware stress, final negative/resize, packages/manifest/SHA, clean extraction smoke, independent audit and Draft PR. PACT-70 remains gated until every P0 and base PR/package gate is green.

## PACT-80 preparation

Added 6 focused boundary cases: invalid format/state, imported WAR and ReadWrite chains, overwrite retention, concrete cycle edge membership, exact 65,536 usage limit and exact 262,144 edge limit plus derived overflow. MQB passed all; total focused unit cases are now 94 (41 compiler + 29 planner + 14 options + 4 shader + 6 boundary).

All required portfolio documents, source-only release policy, documentation/link checker and Windows/Ubuntu CI are present. Added fail-closed scripts for the clean full matrix, local package/manifest/SHA and fresh extraction. Build/document contracts pass and PowerShell scripts parse. These are preparation results; packages, 1000-frame stress, clean extraction, CI, independent audit and Draft PR remain pending until after the preparation commit and clean-head run.

First clean local full run at `cb592de52bf9dd767a1efe3359379aef12ff1218`: all 21 steps passed, including MQB, MSVC Debug/Release 11/11, Clang 10/10, ASan+UBSan 10/10 plus 10k+10k, both parity suites, both 1000-frame stress runs, resize, negatives, primary Inspector, package and fresh-extraction 240-frame WARP. Win64 ZIP 1,291,346 bytes SHA `4c01868092a6c89b9c01f4c3858747a58f8edd805867b751d5b8318dcbdcd4aa`; source ZIP 1,835,587 bytes SHA `60b5aa23b6fb87d6129ce6996624eb80c736d6b5b277c0dd057a0dc9295b617c`. This receipt becomes historical after the audit fix and must be fully rerun.

Independent audit found only the Ubuntu Clang CI blocker and stale progress Medium. Windows CI passed; GCC compiled before matrix cancellation. Follow-up audit rejected conditional Expected selection as a public ABI mismatch. The final staged fix uses one unconditional project-owned layout, warning fix, CI fail-fast change and install/export consumer verification. Final local recertification, green remote CI, audit-fix approval and Draft PR remain pending.

## Certified P0 baseline / PACT-70 entry

At `7f90d9f8815d5c27390fad3f7d62f05b2d273dae`, final local 22-step verification passed (including install consumer and both 1000-frame stress), GitHub CI run 33878631431 passed all four jobs, audit/fix review approved, final packages/extraction passed, and Draft PR #1 was created. Exact P0 artifacts/manifest are under artifacts and remain historical once PACT-70 changes HEAD. PACT-70 entry time was checked against the updated cutoff; user has not frozen features. PACT-70 contract is now active.

## PACT-70 GPU-driven culling

The scene graph now declares InitCulling and GPUFrustumCulling, transient VisibleInstances/IndirectArgs buffers and an imported CullReadback. Core owns the new read-only IndirectArgument usage/state and plans UAV ordering plus UAV→SRV, UAV→Indirect and Indirect→CopySource transitions. The D3D12 executor only maps that state, binds declared UAV/SRV access, executes one indirect pillar draw in both depth/HDR, and reads back the argument count after both consumers. GPU mode is default; `--draw-mode cpu` and G provide the reference path without changing the graph or identity.

The fixed-camera CPU oracle and HLSL both selected 149/160 pillars at frame 0. WARP and NVIDIA hardware each passed CPU-direct/GPU-indirect exact raw RGBA parity, stable per-build plan identity, all 11 timestamped passes and Debug 0/0/0. A first GPU/CPU attempt exposed that `StartInstanceLocation` did not supply the expected source identity to this shader path; explicit per-draw root-constant identity repaired both geometry and color parity. The original executor probe and scene alias-on/off suites remain green on both adapters.

MQB/MSVC Debug and Release app builds pass. Focused totals are 41 compiler, 30 planner, 15 option, 6 boundary, 4 shader and 1 Expected case. MSVC CMake Debug/Release are 12/12 each; portable Clang profiles and ASan+UBSan are 11/11. MQB Release and Sanitizer each passed the 10,000 valid + 10,000 invalid independent full-plan sweep. Development commands/results are in `evidence/pact70-*.log`; exact clean post-commit culling/property evidence will be written to `artifacts/checkpoints/pact70.json`.

The 1280x720 120-frame hardware observation reports logical/reference/actual heap bytes 16,589,456 / 17,825,792 / 13,893,632, saving 3,932,160 (22.06%). GPUFrustumCulling averaged 0.0221 ms locally; this is an observation, not an SLA. PACT-70 implementation checkpoint `984c228c0d6a4829f9fdb81e4538d6ce2a57a788` was pushed with a clean tree. `artifacts/checkpoints/pact70.json` binds MQB Release, 10k+10k, WARP/hardware culling and scene parity plus the 120-frame report to that exact local/remote SHA. Final P0 recertification, package/manifest update, CI, PR refresh and post-change audit remain before final delivery. Tier 2 may now begin with measurement.

## Tier 2 performance and memory hardening

A new cross-platform `framegraph_benchmark` target measures one fixed 64-resource/192-pass graph in compile, allocation alias-on/off and barrier alias-on/off stages. It retains raw per-operation samples, medians, compiler/configuration, graph dimensions, plan identity, plan counts and a live checksum in JSON; CTest validates the schema/invariants. MQB and CMake consume its source from the same build manifest. `benchmark_runtime.py` runs paired policies serially, retains every native report and summarizes medians with adapter/driver/debug/provenance.

Before/after MSVC Release data uses 31 samples × 1,000 operations after warmup. Clean checkpoint measurements put compile at 52.5261→39.5762 µs (-24.65%); alias-on barrier planning at 26.9801→25.3368 µs (-6.09%). The only production change reserves local/output vectors from already validated bounds, using actual retained counts for persistent result capacity. Plan identity stayed `87a12f2fdda63447`; memory and 192 transitions/64 UAV/64 activation barriers are exactly unchanged. Allocation stayed about 2.2 µs and was not modified. Barrier audit found no provably redundant operation, so none was removed.

Seven paired clean Tier 2 hardware runs at 1280x720×240 frames observed 13,893,632 alias-on versus 17,825,792 alias-off actual bytes (-3,932,160, -22.06%). Median graph-work GPU time was 0.103273 versus 0.104190 ms and CPU record was 0.089782 versus 0.091111 ms. Every pair matched pixels/counts and Debug 0/0/0. The small timing deltas do not justify weakening activation safety; no SLA is claimed.

Final reserve code passed MQB Release 100,000 valid + 100,000 invalid independent full-plan graphs and 100,000 bounded mutations. The initial reserve version also passed the 100k+100k sweep under Clang ASan+UBSan; the final refinement passed sanitized CTest 12/12. Current CMake/MSVC Debug/Release are 13/13; portable Clang, the local g++-named LLVM driver and Sanitizer profiles are 12/12. Tier 2 implementation checkpoint `57dfbab8b328ecd644062f881e3a2bef96262584` is pushed; `artifacts/checkpoints/tier2.json` binds clean local/remote SHA, final raw benchmarks and stress. Raw paths, exact environment and limitations are recorded in `evidence/tier2-results.md`. Tier 3 reliability follows.

## Tier 3 reliability hardening

The previous resize switch performed one cycle. A focused RED required five cycles in 100 frames; runtime stress now repeats every 20 frames. WARP and hardware each passed 15 true size changes, 5 minimize/restore pairs, 16 exact graph compilations, final 1280x720 capture quality, CPU/GPU visible-count equality and Debug 0/0/0. Ordinary execution is unchanged because the behavior is behind `--resize-stress`.

A real WARP-device unit target validates the last owned CPU/GPU descriptor indices and rejects the first past-capacity index plus GPU access to a non-visible heap as typed `DescriptorExhausted`. The build manifest now owns a shared arena source group used by both the app and test, and generated MQB/CMake policies remain drift checked. Shader diagnostics remain 4/4; impossible adapter, watchdog, invalid CLI, invalid graph, undeclared access and zero capture deadline all fail closed.

The fence/device review confirms per-frame wait-before-reuse, bounded final/resize/destruction drains, rejection of `UINT64_MAX` before fence increment, and full HRESULT/removal-reason/current-pass/DRED reporting. The real PACT-40 device-removal packet remains evidence; Tier 3 did not deliberately hang a healthy GPU. See `evidence/tier3-fence-device-review.md`, `tier3-resize-red.log` and `tier3-green.log`. Tier 3 implementation checkpoint `2e0c43562a5e9679aaf2d225d9dd192a0845afcf` is pushed; `artifacts/checkpoints/tier3.json` binds clean local/remote SHA, CMake 14/14, both adapter reports and negative results. Remaining: final clean full verification, repeated clean extraction, CI/PR refresh and final independent audit.
