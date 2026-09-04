# Root cause packet: hardware alias parity

Observable symptom: the initial Neon Ruins hardware run rendered valid-looking, Debug-clean images for both policies, but 22,831 pixels / 63,435 RGB bytes differed. WARP was byte-identical. Alias-on repeated identically, ruling out timing noise. The missing pixels appeared in floor/HDR regions after Depth storage was reused for BloomA.

Minimal reproduction: NVIDIA adapter, fixed seed 24301, logical frame 11, 640x360, compare raw RGBA for `--aliasing on` and `off`. The first differing byte was offset 281600; alpha never differed. Plans placed Depth at offset 0, SceneHDR at the adjacent aligned offset, and reused only Depth's dead bytes for BloomA/BloomC. Independent active-interval checks and Debug Layer were green.

Confirmed facts: plans and commands were equal through SceneHDR. Layout intervals did not overlap live resources. The behavioral difference began only after the cross-format Depth→BloomA activation with a named `pResourceBefore`. Repeating the same alias-on plan was deterministic. The backend exactly mapped Core's named barrier, so this was a Core policy boundary rather than backend inference.

Fix: Core retains exact predecessor/successor events for diagnostics but explicitly plans conservative native null-before scope for every activation. The backend maps this plan bit and does not decide scope. A RED/GREEN Core test covers the new native scope and the field enters plan identity. Hardware alias-on/off then became byte-identical in two complete repeats; WARP stayed identical. Each run reports Debug 0/0/0.

This trades potentially broader synchronization for portable correctness in the v0.1 single-queue laboratory. Performance reports measure the resulting policy; no named-barrier optimization claim remains.
