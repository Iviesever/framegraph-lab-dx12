# Final audit follow-up

Independent read-only audit at clean HEAD `624f07b9e8091117aa2999067d216aa14b3df547` found no Blocker or High and four Medium evidence/coverage issues.

1. Default GPU indirect alias-on/off parity was not literally byte-tested because `validate_scene.py` forced CPU. It now explicitly uses `--draw-mode gpu` and still compares complete `.rgba` files. `validate_culling.py` separately keeps GPU↔CPU exact bytes/counts.
2. Fence/device review overstated DRED fields. The claim now matches `diagnostics.cpp`: command-list name, completed breadcrumb operation count and page-fault address; queue/allocation-name traversal is not claimed.
3. Tier 2 table cited the intermediate `core-after.json`; the exact `39.5762 µs` table source is now named as `core-final.json`.
4. Tracked status and Draft PR were stale. Progress/matrix are updated to the exact `624f07b...` 27-step/package/CI/browser evidence. Draft PR #1 is updated only after this follow-up commit passes a new clean full verification, so its final head/artifact hashes cannot drift.

After commit: run WARP and hardware `validate_scene.py`, then `scripts/verify.ps1` from the clean pushed audit-fix HEAD, repeat package extraction, require green CI, refresh the Draft PR, and request audit follow-up approval.
