# Interview guide

**What problem does a render graph solve?** It turns pass intent into a global schedule/resource synchronization/memory plan. Pass code stops hand-maintaining transitions and temporary lifetime ownership.

**RAW, WAR, WAW?** RAW protects a reader from running before the prior writer. WAR keeps a following writer from clobbering data while prior readers need it. WAW gives consecutive writers a defined version order.

**Why stable topology?** Many valid orders exist. Lowest-ready-ID selection makes serialization, debugging, tests and plan caching reproducible.

**How are dead passes selected?** Present, side effects and last writers of exports are roots. Reverse edge closure retains prerequisites; remaining passes cull. v0.1 conservatively retains overwrite-related prerequisites.

**What is a lifetime?** Inclusive first/last retained schedule positions where a resource is required. Exports extend to graph completion.

**Why only non-overlapping lifetimes alias?** Simultaneously live logical resources must own distinct bytes. If A's last position is strictly before B's first, a barrier can transfer one region.

**Logical versus physical allocation?** A logical texture describes content/use. Physical allocation says heap, offset, actual size/alignment and reuse chain. Several logical resources can serially own one slot.

**Committed versus placed resource?** Committed creation combines one implicit heap/allocation with a resource. Placed creation lets this application create explicit ID3D12Heap objects and place multiple resources at planned offsets.

**Transition, UAV and alias barriers?** Transition changes usage state/layout. UAV orders writes while remaining UAV. Aliasing transfers active interpretation of shared heap bytes. None substitutes for another.

**Why is the backbuffer imported?** DXGI creates/owns it outside the graph. The graph borrows its pointer, declares Present initial/final states and never places it in transient heaps.

**Why not compile every frame?** Pass/resource structure does not normally change. Recompile only for resize/config/alias policy; replaying avoids CPU work and identity churn.

**How do fences protect frames in flight?** Each arena/list/allocator has a fence value. CPU waits that value before reusing the frame context; other contexts remain in flight.

**What does WARP prove?** API correctness, shader execution, deterministic automation, capture and Debug behavior on Microsoft's software adapter. It does not prove hardware performance/vendor behavior; hardware exposed the named-alias parity issue WARP missed.

**What does Debug Layer prove?** It detects many invalid descriptors/states/lifetimes and reports warnings. Zero messages does not mathematically prove rendering, data initialization, visual quality or cross-vendor correctness.

**Why aren't timestamps in identity?** They are runtime observations affected by load/adapter/power. Identity represents semantic compiler output only.

**Why no cross-GPU Pixel Hash promise?** Floating-point raster/filter/shader behavior can differ within valid tolerances. On/off parity is byte-exact only on the same adapter/settings/frame.

**Why no async compute?** One direct queue keeps v0.1 ownership/barriers/fences auditable. Queue transfers and overlap would multiply the proof surface.

**How could this connect to UE/RHI?** Translate UE/RHI resource descriptions/states to the Core vocabulary, query RHI allocation constraints, and map the compiled plan back. This independent lab intentionally does not modify UE, emulate UHT or replace UBT.

**What did AI do and what can the user claim?** Codex produced architecture detail, code, tests, diagnostics, capture, package/audit/docs under user-defined product/scope/deadline/acceptance. Present it as AI-assisted engineering. The user should claim product direction and demonstrated understanding after completing a live drill, never independent hand-authorship.

**Why conservative null-before alias scope?** Exact chains stay in the plan, but hardware parity showed named cross-format activation diverged. Null-before produces broader, explicit safety; benchmarks disclose its cost.
