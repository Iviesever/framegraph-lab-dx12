# FrameGraphLab

C++23 render graph compiler and native Direct3D 12 transient-memory laboratory.

```text
Declarative Passes → Dependency + Lifetime Compile → Transient Heap Aliasing
                  → Automatic D3D12 Barriers → HDR / Bloom / Tone Map
                  → GPU Timing + Inspector + Packaged Evidence
```

Version 0.1 is under development. See the [product contract](tasks/20260904-184500-framegraph-dx12-0.1/product_contract.md) and [progress](tasks/20260904-184500-framegraph-dx12-0.1/progress.md) for verified status. No graphics functionality is claimed by this baseline.

## Baseline build

Requires Python 3 for build-contract checks, C++23, and MSVC x64. MQB is the primary local Windows build/run entry. CMake 3.28+/CTest provide cross-platform Core, install/export and CI. Windows runtime additionally requires the Windows SDK.

```powershell
./scripts/build.ps1 core -Configuration debug
./scripts/build.ps1 compiler_tests -Configuration debug -Run
./scripts/build.ps1 planner_tests -Configuration debug -Run
./scripts/build.ps1 property -Configuration release -Run --cases 10000
```

This is AI-assisted engineering. The user defined the product, scope and acceptance requirements; Codex is implementing and verifying the delivery. See the task contract for authorship boundaries.

See [build policy](docs/BUILDING.md) for shared MQB/CMake configuration and portable commands. Local binary packaging and clean-extraction validation remain required delivery gates. A future authorized GitHub release will be source-only; it will not attach a precompiled demo, avoiding large binary transfers. Current graphics/package status remains pending in the progress record.
