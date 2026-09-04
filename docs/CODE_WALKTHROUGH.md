# Code walkthrough

Start at `include/framegraph/graph.hpp`. The description types are plain values; IDs are local dense indices. Read `validation.cpp`, then `compiler.cpp`: the first scan creates reasons, stable Kahn order validates acyclicity, iterative DFS builds a cycle chain, reverse closure retains roots, and schedule positions create lifetimes. `serialization.cpp` emits schema 1 and byte-stable FNV diagnostic identity.

Next read `plan.hpp`, `allocation.cpp`, `barriers.cpp` and `plan.cpp`. The allocator is a stable slot reuse algorithm, not optimal packing. Barrier planner works only from retained usages/allocation; note the conservative native-null alias scope and exact predecessor field. Property tests are intentionally different algorithms.

Native startup flows `main.cpp` → option/report → `runtime.cpp` → `Dx12Context`. `context.cpp` owns Debug/DRED, adapter/device/queue/swapchain, 3 frame contexts, fences and resize. `window.cpp` owns HWND/input. `shader.cpp` owns D3DCompiler diagnostics/executable path.

`arena.cpp` maps descriptors/heap classes, asks allocation requirements, creates real heaps/placed resources and views. `executor.cpp` binds imports, maps Core barriers, initializes RT/DS, guards callbacks, records timestamp pairs and reads per-frame results. `capture.cpp` owns footprint/readback/WIC.

`scene.cpp` builds immutable pipelines and the eight-pass graph. Procedural geometry is in `scene.hlsl`; post effects in `post.hlsl`. The executor probe remains a focused UAV/alias regression. `tools/build_inspector.py` converts final JSON/PNG into one offline HTML.

For a live demonstration: set a breakpoint in `GraphCompiler::compile`, inspect edges/order/lifetimes; continue to `TransientAllocator::plan` and compare Depth/Bloom offsets; inspect `Dx12GraphExecutor::emit`; then toggle A and show identity/memory changing while raw frame bytes remain equal.
