# FrameGraphLab

C++23 render graph compiler and native Direct3D 12 transient-memory laboratory.

```text
Declarative Passes → Dependency + Lifetime Compile → Transient Heap Aliasing
                  → Automatic D3D12 Barriers → HDR / Bloom / Tone Map
                  → GPU Timing + Inspector + Packaged Evidence
```

Version 0.1 is under development. See the [product contract](tasks/20260904-184500-framegraph-dx12-0.1/product_contract.md) and [progress](tasks/20260904-184500-framegraph-dx12-0.1/progress.md) for verified status. No graphics functionality is claimed by this baseline.

## Baseline build

Requires CMake 3.28+, C++23, and MSVC x64 or Clang/GCC. Windows runtime additionally requires the Windows SDK.

```powershell
./scripts/with-msvc.ps1 cmake --preset msvc-debug
./scripts/with-msvc.ps1 cmake --build --preset msvc-debug
./scripts/with-msvc.ps1 ctest --preset msvc-debug
```

This is AI-assisted engineering. The user defined the product, scope and acceptance requirements; Codex is implementing and verifying the delivery. See the task contract for authorship boundaries.
