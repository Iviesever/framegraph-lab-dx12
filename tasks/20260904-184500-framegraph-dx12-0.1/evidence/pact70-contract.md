# PACT-70 GPU-driven culling contract

Entry gate satisfied at certified clean P0 HEAD `7f90d9f8815d5c27390fad3f7d62f05b2d273dae`: local full matrix/package/extraction green, Debug/parity green, CI 33878631431 green, independent audit approved and Draft PR #1 exists. Current UTC/JST is recorded by tool evidence and is before 2026-09-05 10:30 JST; user has not frozen features.

Add Core `IndirectArgument` usage/state with RED/GREEN transition tests. Scene graph always declares InitCulling and GPUFrustumCulling compute passes, VisibleInstances and IndirectArgs buffers, and a CullReadback pass. UAV/Indirect/Copy transitions and UAV ordering come only from Core.

HLSL computes stable visible IDs for 160 procedural pillars at the fixed camera. CPU independently evaluates the same fixed frustum and reports count. Read back indirect args after both depth/scene consumers and fail if GPU count differs. GPU mode binds VisibleInstances to vertex shaders and uses one ExecuteIndirect draw for pillars; CPU mode issues one direct draw per CPU-visible original instance. Floor remains direct in both.

RED acceptance against current app requires `--draw-mode gpu|cpu`, named passes/resources, nonzero Core UAV barriers, GPU/CPU visible-count equality, GPU/CPU exact RGBA parity on the same adapter at one fixed frame, zero Debug messages and report input/visible counts. Test WARP and hardware. Re-run P0 scene/resize/stress/package after integration; update the existing Draft PR. No async queue, occlusion hierarchy, mesh shader, ray tracing, bindless framework or new backend.
