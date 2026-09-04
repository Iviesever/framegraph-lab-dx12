# Architecture

```text
Declarative passes + whole-resource usages
                  ↓
 validation → RAW/WAR/WAW → stable topology → root closure/lifetimes
                  ↓
 actual device allocation requirements → stable slots → Core barriers
                  ↓
              immutable CompiledPlan
                  ↓
 D3D12 enum mapping → per-frame placed arena → guarded callbacks
                  ↓
 Depth → HDR → extract → blur H/V → tone map → capture → present
                  ↓
 plan/report/PNG → offline Inspector → package evidence
```

`framegraph_core` is an OS/GPU/clock/filesystem-independent C++23 library. App depends on the D3D12 layer, which depends on Core; Core never points upward. `GraphCompiler` owns validation, dependencies, culling, schedule and lifetimes. `TransientAllocator` owns byte slots. `ResourceStatePlanner` owns transition/UAV/activation/final lists. Their values form one `CompiledPlan`; the backend has no second hazard or barrier state tracker.

Device allocation size/alignment cannot be known by portable Core. The executor first validates the description with Core, asks the actual device for plain requirements, then calls the same Core allocator/state planner. This composes one plan when size/config/alias policy changes. Ordinary frames replay it. Plan identity includes all semantic inputs/outputs and excludes SHA, timing and wall clock.

Each frame context has its own command allocator/list/fence value and `Dx12PlacedResourceArena`. CPU waits only before reusing that context. Imports borrow the acquired backbuffer/readback; transient heaps/resources/descriptors remain per frame. Resize waits all contexts, destroys executors that borrow old backbuffers, calls ResizeBuffers, rebuilds and recompiles exactly once.

Executor callbacks receive `Dx12PassContext`, which checks IDs and operation usage against the retained pass. Before callback invocation it records Core barriers in order and initializes newly activated RT/DS metadata. After callback it records epilogues/timestamp. Instrumentation query/readback buffers have fixed states and do not introduce hidden application-resource decisions.

The scene renderer owns immutable shader/root-signature/PSO objects. Graph recreation reuses them. Scene state owns logical frame, camera and debug view. Physical frame count controls bounded automation; pause stops logical advancement. Capture maps only after the selected frame fence.

Build ownership is similarly one-directional: `build-manifest.json` → generated `mqb.json` and CMake target properties. MQB is the local Windows authority; CMake/CTest exports portable Core and CI. See [building](BUILDING.md), [compiler](RENDER_GRAPH_COMPILER.md), [memory](TRANSIENT_MEMORY.md), [barriers](RESOURCE_BARRIERS.md) and [backend](D3D12_BACKEND.md).
