# FrameGraphLab 0.1.0

This candidate delivers a deterministic portable C++23 render-graph compiler and native D3D12 executor with actual placed-resource reuse, automatic whole-resource barriers, GPU frustum culling and ExecuteIndirect, hardware/WARP parity, GPU timings, procedural HDR/bloom demo and offline Inspector.

Highlights: stable RAW/WAR/WAW/topology/cycle/dead-pass/lifetimes; typed validation; canonical schema 1; independent 100k+100k oracles and 100k bounded mutation; class/alignment/overflow-safe transient slots; per-frame heaps/fences; graph-managed UAV/SRV/Indirect transitions; CPU/GPU culling-count and pixel parity; Debug/DRED diagnostics; WIC RGBA capture; actual 22.06% hardware heap saving at 1280x720; byte-exact same-adapter alias parity; self-contained interactive evidence.

Reliability gates repeat 15 resizes and five minimize/restore cycles per adapter, validate descriptor exhaustion on a real WARP device, retain shader/adapter/watchdog/graph/access/capture negatives, review fence wrap/lifetime and preserve the earlier real device-removal diagnostic packet.

The GitHub Release publishes the verified Win64 ZIP, its SHA-256 file, and Delivery Manifest alongside GitHub-generated source ZIP/tar.gz. The binary package passed the full clean-head matrix and two independent fresh-extraction WARP smokes. PDBs, loose compiled shaders, separate evidence bundles, and a duplicate manual source ZIP are not uploaded.

See [known limitations](KNOWN_LIMITATIONS.md), [building](BUILDING.md), [testing](TESTING.md), and [AI assistance](AI_ASSISTANCE.md). PACT-70 deliberately excludes async compute, mesh shaders, ray tracing and occlusion hierarchy.
