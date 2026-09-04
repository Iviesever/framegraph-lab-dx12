# Neon Ruins scene

Neon Ruins is generated entirely in HLSL. There are no texture, mesh or model assets. `SceneVS` expands six floor vertices and 36 cube vertices through SV_VertexID, then places 160 pillars through SV_InstanceID and a stable integer hash of the scene seed. An orbit camera and moving point light change from the logical frame. The floor shader draws an emissive procedural grid; pillars alternate cyan/magenta and brighten toward their tops.

The same vertex construction runs in DepthPrepass and SceneHDR. DepthPrepass writes a D32 attachment after executor initialization. SceneHDR binds a read-only DSV, uses less-equal depth testing and writes R16G16B16A16_FLOAT. The next three full-screen passes extract pixels brighter than one and run a nine-tap separable Gaussian blur at half resolution. ToneMap combines full-resolution HDR with bloom, applies an ACES-style curve and gamma, and fills the imported RGBA8 backbuffer. Capture and Present close the graph.

Core lifetime planning finds two reuse events at 1280x720: Depth→BloomA and BloomA→BloomC share one offset. HDR and BloomB remain in separate simultaneous slots. The hardware run reserves 13,762,560 bytes per frame arena instead of 17,694,720 reference bytes, saving 3,932,160 bytes (22.22% of the alias-off reference). Logical unpadded texture/buffer bytes are reported separately. WARP may return slightly different allocation sizes.

Aliasing policy changes heap placement and plan identity only. It does not change scene seed, logical frame, camera, shader constants or draw calls. Hardware and WARP each pass byte-exact on/off readback comparisons; hashes are not promised across adapters. The D3D12 plan uses conservative null-before activation for all placed resources and retains exact predecessor/successor events. This policy closed a hardware-only cross-format parity failure and remains explicit in schema 1.

Every pass has a timestamp pair and CPU record observation. Capture copy time is included and often dominates this compact workload; timing is a local observation, not an SLA. Resize recreates the size-dependent graph and three arenas only after fences. Ordinary frames do not recompile.

Controls: left/right arrow or left mouse drag rotates; Space pauses logical animation; N single-steps; R resets; A drains fences and toggles aliasing; V cycles Final, HDR-only and Bloom-only tone-map views. The title displays logical frame, alias policy and view. Headless runs use fixed inputs and exit at the requested physical frame count.
