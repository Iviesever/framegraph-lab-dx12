# Known limitations

- Whole-resource state tracking only; no subresource ranges or enhanced barriers.
- One direct queue; no async compute, ownership transfer or multi-GPU.
- Stable explainable slot reuse; no optimal packing, splitting or residency manager.
- Default 64-KiB placement, no MSAA/4-MiB or small-resource 4-KiB alignment path.
- Legacy D3D12 barriers with conservative null-before alias activation; exact chains remain diagnostic.
- D3D12/Win32 only; Core is portable but there is no Vulkan/OpenGL/Metal/engine integration.
- D3DCompiler shader model 5 pipeline; no DXC, hot reload, bindless, mesh shaders or ray tracing.
- Procedural scene only; no asset pipeline, PBR, ECS, editor or ImGui.
- Debug view exposes Final/HDR/Bloom; no arbitrary texture inspector or depth visualization.
- Pixel equality is only asserted between alias modes on the same adapter/settings/logical frame.
- GPU timings are local observations. Capture copy is included and can dominate small workloads.
- DRED is enabled and reported if device removal occurs; validation does not intentionally hang/remove a GPU.
- LLVM-MinGW's driver does not support local coverage-guided libFuzzer mode; bounded fuzz and ASan+UBSan structured sweeps are used.
- Static HTML uses modern browser APIs including Map.groupBy; browser QA targets current Edge/in-app Chromium.
- GitHub CI builds/tests Core and Windows code but does not claim hosted GPU validation.
- Local binary/source packages are required and validated, but binaries are never attached to the source-only release policy.
- Culling handles the fixed 160-pillar demo with one direct-queue compute group and stable prefix compaction. There is no occlusion hierarchy, async compute, GPU scene system or variable-size indirect command stream.
- `Expected<T,E>` implements only the value/error/void operations used by this project; it is not a drop-in implementation of every `std::expected` convenience API.
