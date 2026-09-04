# Final Inspector browser QA

Source/artifact SHA: `3279c8b1c4521160c5ea1ac1b2d993d04b481edb`. Inspector: `artifacts/viewer/framegraph-inspector.html`, 715,194 bytes. Plan identity `212d2a2c9156afb0`; 11 active passes, 10 resources, 19 dependency edges; embedded PNG SHA-256 `c00de55008990f99a47a51ee95919910708f122e09b7f6e87fc0a0a54652da65`.

Actual in-app Chromium QA used a temporary localhost server rooted only at `artifacts/`, then closed all tabs and stopped the owned server.

- Desktop 1280×720: document horizontal overflow 0; browser console error/warning list empty. Screenshot, memory, identity, DAG, lifetimes, heaps, barriers and timings rendered legibly.
- Selecting DepthPrepass updated details to Depth Write/DepthWrite, VisibleInstances Read/ShaderRead and IndirectArgs Read/IndirectArgument. Its barrier panel showed `R5 UAV → ShaderRead` and `R6 UAV → IndirectArgument`.
- Selecting IndirectArgs highlighted the five related passes and showed its full activation/Common→UAV, UAV ordering, UAV→IndirectArgument, IndirectArgument→CopySource and CopySource→Common chain.
- A same-origin explicit iframe supplied a real 375px content viewport. `matchMedia('(max-width: 760px)')` was true; content/document client widths were 360/360, document scroll width 360 and horizontal document overflow 0. The DAG's own horizontal track remained independently scrollable as intended.
- Narrow selection of GPUFrustumCulling updated the detail to both buffers as ReadWrite/UAV and the two UAV ordering barriers. Browser console remained empty.

One discarded implicit-markup harness produced a CUA-injected MutationObserver error; the Inspector source contains no MutationObserver. Repeating with explicit html/head/body/iframe markup yielded zero logs and is the recorded result.
