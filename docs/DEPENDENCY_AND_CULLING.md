# Dependencies, ordering and culling

For each resource, declaration order defines a sequence of accesses. RAW orders a reader after the last writer. WAR orders the next writer after outstanding readers. WAW orders consecutive writers. A UAV ReadWrite usage has both RAW and WAW dependency reasons. Edge records keep both resource and hazard provenance even when several reasons connect the same two passes. Adjacency deduplicates pass pairs before computing indegree.

Stable Kahn traversal always chooses the smallest ready PassId. Sorting and deduplicating usage and explicit-edge arrays removes irrelevant input ordering. An iterative colored DFS supplies a closed pass chain when topological traversal cannot consume all passes; diagnostics include names and IDs. Iterative traversal avoids recursion depth dependence.

Roots are side effects, Present usages, and the last writer of each exported resource. Reverse dependency closure retains their prerequisites. Other passes cull, and retained passes retain their relative stable topological order. This v0.1 policy is deliberately conservative: WAW/WAR prerequisites can retain an earlier overwritten producer and its readers. It does not claim optimal version-aware dead-store elimination.

Lifetimes use inclusive retained schedule positions. A resource first and last used in pass 3 lives at [3,3]; another beginning in pass 3 cannot share those bytes. Exported outputs extend to the final retained pass because the caller consumes them after execution. Culled-only resources have no lifetime or allocation. Unused imports can still have a required final state, handled by the plan's final barriers.

The property oracle uses pairwise access relationships and brute-force ready selection instead of the production last-writer scan and priority queue. It independently checks edge reasons, culling, order and intervals.
