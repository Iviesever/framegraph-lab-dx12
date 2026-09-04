# Benchmarking

Timing fields are local observations after a bounded fixed-seed run. They are not cross-machine SLAs. Compare alias modes on the same executable, adapter, resolution, frame count, camera/logical frame and Debug policy. Keep raw reports and report median/repeated samples before optimizing.

The verified NVIDIA RTX 4070 Laptop GPU 1280x720, 120-frame PACT-70 Debug-Layer run observed these mean GPU times: InitCulling 0.0062 ms, GPUFrustumCulling 0.0221, DepthPrepass 0.0227, SceneHDR 0.0171, BloomExtract 0.0091, horizontal/vertical blur 0.0098/0.0113, ToneMap 0.0257, culling readback 0.0032, Capture 0.4420 and Present 0.0232. Capture includes full-frame copy and dominates this small workload. Values may change with power/driver/load.

Memory is clearer: 16,589,456 logical bytes; 17,825,792 device requirements/reference bytes; 13,893,632 actual heap bytes per frame with aliasing; 3,932,160 saved (22.06%). Three frame arenas reserve 41,680,896 resource-heap bytes. This excludes descriptors, command objects, swapchain, readback/query buffers, driver allocation and residency. It is evidence of actual CreateHeap/PlacedResource offsets, not total VRAM telemetry.

The Tier 2 fixed graph has 64 transient buffers and 192 retained passes/usages. With 31 samples of 1,000 operations after warmup, MSVC Release medians moved from 52.5261 to 39.5762 µs for compile (-24.65%) and 26.9801 to 25.3368 µs for alias-on barrier planning (-6.09%). The change only reserves vectors from validated bounds. Plan identity, memory and all 192 transitions/64 UAV/64 activation barriers remain exact. Allocation was already about 2.2 µs and its algorithm was not changed. Raw samples are retained under `artifacts/benchmarks`; the tracked result record explains the environment and limits.

Seven paired 240-frame hardware samples observed identical 16,589,456 logical bytes in both policies and 13,893,632 versus 17,825,792 actual heap bytes. Alias-on therefore removed 3,932,160 bytes (22.06%). Median GPU graph work was 0.103273 ms on versus 0.104190 ms off; CPU graph record was 0.089782 ms on versus 0.091111 ms off. These small timing deltas did not justify weakening the conservative activation policy. The independent oracle also completed 100,000 valid plus 100,000 invalid graphs, and bounded mutation completed 100,000 iterations.

Suggested reproducible comparison:

```powershell
./scripts/build.ps1 app -Configuration release
./.mqb/bin/FrameGraphLab-release.exe --hardware --headless --frames 1000 --aliasing on  --report artifacts/reports/hardware-on.json
./.mqb/bin/FrameGraphLab-release.exe --hardware --headless --frames 1000 --aliasing off --report artifacts/reports/hardware-off.json
```
