# PACT workflow

1. Read the task contract and next incomplete acceptance item.
2. State the expected behavior and create a focused failing test before production changes.
3. Record RED, implement the minimum behavior, record GREEN and relevant regression.
4. After two failed repairs, write a root-cause packet before a different evidence-based repair.
5. Update progress with exact tested revision, commands, exit codes, graphics status and limitations.
6. Commit each completed PACT, push, and record a recoverable last-known-good revision.
7. On pause, finish the atomic operation, stop owned long-running processes, write reset_handoff and push.
8. Resume by reading AGENTS/contract/progress/handoff, fetching, comparing local/remote HEAD, and continuing.
9. Completion requires all P0 gates, independent read-only audit, clean HEAD packages, and a Draft PR. Never merge.
