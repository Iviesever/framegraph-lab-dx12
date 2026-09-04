# Compiler acceptance cases (before production code)

Every named case below must first fail against the compile stub. Groups may be run together, preserving individual results.

1. Empty graph compiles to empty schedule; invalid ResourceId and invalid explicit PassId fail InvalidHandle.
2. Zero dimensions, impossible format/flags, zero buffer size and extreme size fail InvalidDescriptor or Overflow; bounded graph/name/usage counts fail LimitExceeded.
3. Unknown access/usage enums and incompatible access/state pairs fail InvalidUsage. One resource may appear once per pass; duplicate/conflict fails DuplicateUsage.
4. A transient read before any write fails UninitializedRead, even in a dead pass. Imported initialized resources can be read in their initial state; an explicitly uninitialized import cannot.
5. Writer→reader produces RAW; reader(s)→next writer produces WAR; writer→next writer produces WAW. Edges preserve stable reason and resource provenance. Multiple readers survive independently.
6. Stable topology selects the lowest available pass ID. Reordered explicit-edge input does not change canonical plan. An explicit backedge gives Cycle with a closed concrete pass chain.
7. Exported last writer, Present and side-effect passes are roots; unused branches cull. Overwrite-related retention is deliberately conservative. Inclusive first/last usage is computed only on retained schedule.
8. Required imported final states remain in the compiled source. Unwritten exported transient fails initialization validation.
9. Canonical serialization is JSON, escapes all control bytes and names, orders every collection deterministically, and repeats byte-for-byte. Identity hashes only canonical semantic payload, never Git SHA/timing metadata.

Allocator tests will supply independent lifetime/size/alignment fixtures, verify every pair at each active schedule position, inspect all reused slot chains and simulate state before/after every barrier without calling production planner helpers.
