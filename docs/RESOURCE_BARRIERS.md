# Whole-resource barriers

The Core planner emits pass-before, pass-after and graph-final barrier lists. The backend maps enums and records them in that order; it must not infer dependencies or invent additional state transitions.

- Transition: changes one whole resource from its tracked state to the state required by the declared usage. Consecutive equal native states emit no transition. Common and Present are explicitly equivalent under legacy D3D12's zero state.
- UAV: orders two uses remaining in UAV state whenever either prior or current access writes. Read/read has no UAV barrier. A state transition supplies ordering when leaving UAV state.
- Aliasing: activates each placed resource before its first transition and callback, following the simple activation model. The plan keeps the exact predecessor/successor event but explicitly requests native null-before scope for conservative cross-format activation. Full RT/DS initialization follows before drawing. Activation-barrier count and actual memory-reuse event count are distinct, including in reference mode.

Imports begin in their declared initial states and finish in their required final states, including imports with no retained usage. Backbuffers must request Present. All transient resources return to Common at their last lifetime position before a different occupant activates. A reused slot begins each replay with a null-before activation; per-frame fences protect the arena from simultaneous reuse.

Caller-owned imports must really be in their declared initial states when the plan is executed. A plan with differing imported initial/final states is not automatically repeatable unless the caller reestablishes the initial state. The demo uses matching initial/final Present backbuffers.

This is a legacy whole-resource state model: no subresource tracking, enhanced barriers, queue ownership transfers or async compute. Aliasing barriers and transitions solve different problems; one cannot replace the other. A Debug Layer run remains necessary because Core planning tests cannot validate device commands, descriptor bindings or shader behavior.

Primary API references: [Microsoft resource barrier guidance](https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12), [memory aliasing](https://learn.microsoft.com/en-us/windows/win32/direct3d12/memory-aliasing-and-data-inheritance).
