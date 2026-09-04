# Architecture decisions

## Alternatives considered

1. Stable greedy reusable slots plus a single direct queue (selected): simple independent safety oracle, predictable barriers and exact replay. Does not claim optimal packing.
2. Interval graph coloring with variable-region splitting: potentially better memory savings, larger overlap/alias proof surface. Deferred.
3. Runtime state inference: convenient binding API but duplicates compiler decisions. Rejected by product contract.

## Dependency direction

App/shaders → D3D12 runtime/executor → framegraph Core. Tools/viewer consume serialized Core/runtime records. Core has no OS, GPU, clock, filesystem or unordered iteration dependence. Sequential numeric IDs belong to one GraphDescription; bounds and duplicate declarations are checked. Each resource has exactly one usage per pass; explicit ReadWrite UAV represents combined access.

## Compiler

Pass declaration order defines logical resource versions and RAW/WAR/WAW hazards. Explicit edges support non-resource ordering and diagnostic cycles. First validate the whole description, including dead passes. Stable Kahn ordering chooses lowest PassId. Detect cycles before culling and report a concrete closed chain. Exported resource's last writer, present passes and side-effect passes are roots; retain reverse dependency closure (conservative for overwrite dependencies). First/last schedule positions are inclusive. Imported state and initialization are explicit.

## Memory

Backend obtains device-specific size/alignment via GetResourceAllocationInfo and supplies plain MemoryRequirement to Core. Core creates stable class/compatibility-separated heaps and slots. Reuse requires previous last use < next first use, compatible class/key, size fit, alignment fit and no dedicated requirement. Aliasing off uses disjoint slots. Both use placed resources. Per-frame arena instances avoid cross-frame memory hazards. First activation of a reused slot uses a null-before alias barrier to cover the prior frame; subsequent activations name predecessor/successor. Per-use state transitions and UAV barriers are generated only by Core. Resource last-use epilogues return transient resources to Common before deactivation so recurring frames replay the same plan.

## Execution

Three frame contexts each own allocator, fence value, descriptors, timestamp/readback buffers and transient arena. Wait only before reusing that context; resize/destruction wait outstanding fences. Executor maps plan enums to D3D12 barriers, invokes pass callbacks through a declared-resource guard, and records timestamps. `IndirectArgument` remains a Core state and is only mapped to the native D3D12 state at this boundary. RT/DS resources clear fully upon activation. Imported backbuffers stay outside heaps and finish Present. Graph compilation happens only for size/config/policy changes.

## Scene and observability

Compute frustum culling writes stable visible IDs and one indirect draw argument before instanced procedural pillars and emissive geometry, depth prepass, HDR scene, bloom extraction and separable blur, tone mapping and imported present. A CPU reference computes the same fixed frustum independently and supports direct-draw pixel parity. Fixed seed/camera/frame makes capture comparable on one adapter. WIC receives completed RGBA readback. Reports hold culling count/mode plus hardware/timing/debug metadata; canonical identity excludes wall clock and Git provenance. Static HTML embeds plan/report/PNG and needs no CDN/server.

## Failure and evidence

Typed graph/runtime categories; shader file/entry/target/compiler diagnostics; bounded fence/capture watchdogs; device-removed HRESULT/reason/current pass. Packages rebuilt from clean HEAD into ignored artifacts, with external SHA manifests to avoid self-referential hash claims. Checked-in sample provenance remains truthful if source HEAD changes.
