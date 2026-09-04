# PACT-20 independent read-only review

No confirmed High/Blocker in slot overlap, alignment/overflow arithmetic, immediate predecessor chains or generated state transitions. Reviewer checked variable-size large/small/large reuse against Microsoft's whole-overlapping-resource activation semantics and did not identify a defect.

Confirmed Medium in test oracle: swapping alias activation and transition still passed. Added a mutation regression, observed RED, then added independent active-resource tracking. The same mutation now rejects. Another mutation confirms simultaneous live byte overlap rejects. Planner suite now has 29 cases.

Integration boundaries resolved before backend work:

1. All placed resources now receive first-use activation, including reference mode. This selects the explicit simple activation model. New reference/standalone activation tests failed, then passed. Actual reuse events are reported separately from activation barriers.
2. Imports are execution-state preconditions; differing initial/final states need restoration before replay. Backbuffers will use matching Present states.
3. Generic CPU alignment is not a native heap alignment. Backend will use default 64-KiB resource placement, validate actual returned requirements and create matching heaps. No 4-KiB small-resource or MSAA support is claimed.

Relevant primary references: https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createplacedresource and https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_heap_desc . Final full-project audit remains pending.
