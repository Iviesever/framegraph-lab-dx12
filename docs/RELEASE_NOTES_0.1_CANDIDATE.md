# FrameGraphLab 0.1.0 candidate

This candidate delivers a deterministic portable C++23 render-graph compiler and native D3D12 executor with actual placed-resource reuse, automatic whole-resource barriers, hardware/WARP parity, GPU timings, procedural HDR/bloom demo and offline Inspector.

Highlights: stable RAW/WAR/WAW/topology/cycle/culling/lifetimes; typed validation; canonical schema 1; independent 10k+10k oracles; class/alignment/overflow-safe transient slots; per-frame heaps/fences; Debug/DRED diagnostics; WIC RGBA capture; actual 22.22% hardware heap saving at 1280x720; byte-exact same-adapter alias parity; self-contained interactive evidence.

Distribution policy is source-only on GitHub. Local Win64 and source snapshots are still built, SHA-256-manifested and clean-extraction-tested. To avoid large binary transfer, a future explicitly authorized release will attach no precompiled demo, EXE/PDB/shader binary/capture bundle/report bundle or manually re-uploaded source ZIP. It consists only of an annotated tag, notes, and GitHub-generated source ZIP/tar.gz. This task does not create that tag or release.

See [known limitations](KNOWN_LIMITATIONS.md), [building](BUILDING.md), [testing](TESTING.md), and [AI assistance](AI_ASSISTANCE.md). PACT-70 status is recorded after P0 closure; it never includes async compute, mesh shaders, ray tracing or occlusion hierarchy.
