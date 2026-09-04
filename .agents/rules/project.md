# Scope and ownership

- Work only inside this repository. Do not modify SeedForge, RollbackLab, MQB, UE or any other project.
- C++23; CMake 3.28+; pure Core must not include Windows, Direct3D or COM.
- Backend consumes the Core plan; no second dependency/barrier planner.
- Whole-resource, one direct queue, deterministic plans; no subresource or async-compute claims.
- MQB is the primary local Windows C++/D3D12 build, incremental and run entry. Maintain real mqb.json. CMake/CTest remain authoritative for portable Core, install/export and CI. Clang/GCC are compatibility/sanitizer evidence. Share a verified source/policy inventory; never leave undetected MQB/CMake drift.
- All scratch, shaders, binaries, reports and captures remain inside this repository.
- At most two non-overlapping review subagents. Prefer read-only review. One GPU validation process at a time.
- Never stop unknown processes. Defer long GPU validation while another UE/GPU task is active.
- Commit/push are authorized. Tags and official releases require explicit user authorization; v0.1.0 binary release is explicitly authorized in the current session. Global configuration changes remain unauthorized.
- Generated artifacts must report a clean exact source HEAD. Do not claim a later documentation commit generated earlier artifacts.
- This is AI-assisted engineering; do not claim the user hand-wrote the implementation.
- This is the final new repository in this goal. No other project/repository may be created.
- Keep generated binaries and bundles ignored under `artifacts/`. Draft PRs list local evidence without attachments. The explicitly authorized v0.1.0 GitHub Release uploads the verified Win64 ZIP, its SHA-256 file, and Delivery Manifest alongside GitHub automatic source archives; do not upload PDBs, loose shader binaries, or a duplicate manual source ZIP.
- Updated freeze: 2026-09-05 12:00 JST; internal stop 16:00 JST; deadline 16:30 JST. Reset preparation starts on user's request, never on a guessed usage percentage.
- PACT-70 may start only after all P0/package/debug/parity/Draft PR baseline gates are green, before 2026-09-05 10:30 JST and before user freeze. Then measured performance/memory and reliability work; no unrelated features.
- MQB is not a complete UE build entry without evidence for .uproject/.Build.cs/.Target.cs/UHT/UBT. UE targets remain UBT/UHT; external C++ SDKs may use MQB. Do not implement UE work here.
