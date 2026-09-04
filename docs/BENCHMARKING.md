# Benchmarking

Timing fields are local observations after a bounded fixed-seed run. They are not cross-machine SLAs. Compare alias modes on the same executable, adapter, resolution, frame count, camera/logical frame and Debug policy. Keep raw reports and report median/repeated samples before optimizing.

The verified NVIDIA RTX 4070 Laptop GPU 1280x720, 120-frame Debug-Layer run observed these mean GPU times: DepthPrepass 0.0165 ms, SceneHDR 0.0211, BloomExtract 0.0075, horizontal/vertical blur 0.0090/0.0090, ToneMap 0.0218, Capture 0.3079 and Present 0.0015. Capture includes full-frame copy and dominates this small workload. Values may change with power/driver/load.

Memory is clearer: 16,588,800 logical bytes; 17,694,720 device requirements/reference bytes; 13,762,560 actual heap bytes per frame with aliasing; 3,932,160 saved (22.22%). Three frame arenas reserve 41,287,680 resource-heap bytes. This excludes descriptors, command objects, swapchain, readback/query buffers, driver allocation and residency. It is evidence of actual CreateHeap/PlacedResource offsets, not total VRAM telemetry.

Core benchmarks should measure graph compile, slot allocation and barrier planning independently with a fixed serialized description. Warm up, retain every raw sample, report median and environment/compiler/config. A future optimization requires a measured hotspot, a before/after invariant test and no barrier removal without the independent state oracle. The conditional post-P0 tier may add 100,000 bounded graph stress; it must not invent an SLA.

Suggested reproducible comparison:

```powershell
./scripts/build.ps1 app -Configuration release
./.mqb/bin/FrameGraphLab-release.exe --hardware --headless --frames 1000 --aliasing on  --report artifacts/reports/hardware-on.json
./.mqb/bin/FrameGraphLab-release.exe --hardware --headless --frames 1000 --aliasing off --report artifacts/reports/hardware-off.json
```
