# Benchmarking

Timing fields are local observations after a bounded fixed-seed run. They are not cross-machine SLAs. Compare alias modes on the same executable, adapter, resolution, frame count, camera/logical frame and Debug policy. Keep raw reports and report median/repeated samples before optimizing.

The verified NVIDIA RTX 4070 Laptop GPU 1280x720, 120-frame PACT-70 Debug-Layer run observed these mean GPU times: InitCulling 0.0062 ms, GPUFrustumCulling 0.0221, DepthPrepass 0.0227, SceneHDR 0.0171, BloomExtract 0.0091, horizontal/vertical blur 0.0098/0.0113, ToneMap 0.0257, culling readback 0.0032, Capture 0.4420 and Present 0.0232. Capture includes full-frame copy and dominates this small workload. Values may change with power/driver/load.

Memory is clearer: 16,589,456 logical bytes; 17,825,792 device requirements/reference bytes; 13,893,632 actual heap bytes per frame with aliasing; 3,932,160 saved (22.06%). Three frame arenas reserve 41,680,896 resource-heap bytes. This excludes descriptors, command objects, swapchain, readback/query buffers, driver allocation and residency. It is evidence of actual CreateHeap/PlacedResource offsets, not total VRAM telemetry.

Core benchmarks should measure graph compile, slot allocation and barrier planning independently with a fixed serialized description. Warm up, retain every raw sample, report median and environment/compiler/config. A future optimization requires a measured hotspot, a before/after invariant test and no barrier removal without the independent state oracle. The conditional post-P0 tier may add 100,000 bounded graph stress; it must not invent an SLA.

Suggested reproducible comparison:

```powershell
./scripts/build.ps1 app -Configuration release
./.mqb/bin/FrameGraphLab-release.exe --hardware --headless --frames 1000 --aliasing on  --report artifacts/reports/hardware-on.json
./.mqb/bin/FrameGraphLab-release.exe --hardware --headless --frames 1000 --aliasing off --report artifacts/reports/hardware-off.json
```
