# Tier 2 performance and memory contract

PACT-70 clean checkpoint is green. Tier 2 adds measurement and bounded stress before any production optimization.

The Core benchmark uses one deterministic 64-resource/192-pass buffer graph. Every resource has UAV write, UAV read/write and shader-read stages, creating stable dependencies, transitions, UAV ordering and serial lifetime reuse. Fixed device-style requirements vary size and compatibility. Measure `GraphCompiler::compile`, `TransientAllocator::plan` with aliasing on/off, and `ResourceStatePlanner::plan` with aliasing on/off separately. Warm up first; retain every per-operation sample, median, graph/plan identity, barrier/allocation counts, checksum, compiler/configuration and graph dimensions in JSON. No wall time enters plan identity and no cross-machine SLA is asserted.

The native comparison runs one clean MQB Release executable serially on the same hardware adapter, resolution, seed, draw mode and frame count. Keep every raw alias-on/off report. Summarize median logical/reference/actual/saved bytes, total pass GPU time and total CPU record time, plus adapter/driver/debug/count provenance. Differences are observations, not an SLA.

Run the independent structured oracle with `--cases 100000`, meaning 100,000 valid and 100,000 invalid graphs, and the bounded mutation harness with 100,000 iterations. Both must finish under existing hard input bounds without crashes, sanitizer findings or unbounded waits.

Only change production allocation/barrier/compiler code if these measurements identify a concrete hotspot or a provably redundant operation. Any removed allocation or barrier requires before/after raw samples and the existing independent plan oracle. A green measurement that shows no justified change is a valid result; invented optimization is not.
