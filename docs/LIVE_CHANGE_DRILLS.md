# Live change drills

Complete at least one before presenting the project. For every drill: state the expected plan change, write a focused test, run MQB/CMake regression, explain the result and revert or commit intentionally.

1. Change Bloom threshold from 1.0 to 1.5; predict pixels/timing versus unchanged plan identity (shader behavior is outside plan).
2. Add a third debug view that shows HDR+bloom before tone curve; test V cycling and screenshot distribution.
3. Rename BloomA; prove only canonical identity/Inspector labels change, not offsets/pixels.
4. Add an unused transient pass; demonstrate it appears in culled passes with no allocation.
5. Mark that pass side-effect; explain the new schedule/lifetime and observed GPU timestamp.
6. Swap declaration order of independent passes; predict stable-ID effects and preserve dependencies.
7. Add one explicit ordering edge; inspect canonical dependency reason and cycle negative.
8. Change bloom to quarter resolution; measure allocation savings, GPU timing and pixel differences without an SLA claim.
9. Force BloomC dedicated; predict lost reuse and compare actual heap bytes.
10. Add a second UAV write in the executor probe; identify the exact UAV barrier and run Debug validation.
11. Add a new imported texture with mismatched initial/final states; explain why unchanged replay needs restoration.
12. Implement a safe new whole-resource Copy pass end-to-end: RED compiler fixture, callback guard, D3D run, plan/Inspector/docs.
13. Diagnose a deliberately wrong final backbuffer state using Debug output, then restore the Core plan rule.
14. Add a legal transient buffer to the scene and prove heap-class isolation under Tier 1 hardware rules.
15. Extend one property generator distribution and demonstrate the independent oracle catches a seeded corruption.

For interview readiness, explain graph compile, lifetime, alias transfer, each barrier type, per-frame fences and WARP limits without reading the guide.
