# PACT-50 scene acceptance before production changes

Make a 1280x720 procedural Neon Ruins graph the default application. It must execute DepthPrepass → SceneHDR → BloomExtract → BloomBlurHorizontal → BloomBlurVertical → ToneMap → Capture → Present through the PACT-40 executor. No external art assets; only repository HLSL source.

Write a validation script first that requires all named passes, real depth/HDR/half-resolution bloom resources, nontrivial pixels (broad nonblack ratio, color buckets, luminance range), alias-on/off raw RGBA parity on WARP and hardware, zero Debug messages, GPU samples for every pass and valid PNG. The current solid probe must fail the visual/pass requirements.

Scene geometry: procedural floor grid and at least 128 instanced pillars/cubes from SV_VertexID/SV_InstanceID, orbit camera and moving light, emissive high-contrast palette. Depth prepass writes D32; SceneHDR reads a read-only depth view and writes R16G16B16A16_FLOAT. Bloom uses threshold and separable full-screen blur; tone map combines HDR+bloom into imported swapchain RGBA8. Fullscreen passes cover every pixel.

Callbacks only use declared resources. Core creates all state/alias decisions. Newly activated RT/DS metadata is fully initialized before rendering. Source descriptor identities make non-overlap opportunities explicit; report real logical/committed/actual/saved bytes and all pass timings.

Input: left/right or mouse orbit, Space pause, N step, R reset, A alias on/off (drain fences and recompile one plan), V HDR/bloom debug view. Normal frames do not recompile. Resize drains executor, resizes swapchain, rebuilds size-dependent resources and increments plan_compile_count. Headless capture uses fixed seed/camera/logical final frame.

Shader lookup defaults to executable-relative `shaders`; an explicit shader directory remains typed. Build manifest owns the HLSL asset list. MQB's project-local build script and CMake post-build copy the same list; missing assets fail closed. Packaging later proves no developer-tree lookup.
