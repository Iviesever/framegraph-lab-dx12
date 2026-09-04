# Scope and ownership

- Work only inside this repository. Do not modify SeedForge, RollbackLab, MQB, UE or any other project.
- C++23; CMake 3.28+; pure Core must not include Windows, Direct3D or COM.
- Backend consumes the Core plan; no second dependency/barrier planner.
- Whole-resource, one direct queue, deterministic plans; no subresource or async-compute claims.
- Prefer the installed MQB for local MSVC commands. Preserve required CMake/CTest portability.
- All scratch, shaders, binaries, reports and captures remain inside this repository.
- At most two non-overlapping review subagents. Prefer read-only review. One GPU validation process at a time.
- Never stop unknown processes. Defer long GPU validation while another UE/GPU task is active.
- Commit/push and Draft PR are authorized. Merge, tags, official releases and global configuration changes are not.
- Generated artifacts must report a clean exact source HEAD. Do not claim a later documentation commit generated earlier artifacts.
- This is AI-assisted engineering; do not claim the user hand-wrote the implementation.
