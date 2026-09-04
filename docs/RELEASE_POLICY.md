# Release artifact policy

Generated outputs remain ignored under `artifacts/`. The explicitly authorized v0.1.0 GitHub Release contains an annotated tag, release notes, GitHub automatic source archives, the clean-head Win64 ZIP, its SHA-256 file, and `DELIVERY_MANIFEST.json`.

The Win64 ZIP includes the executable, HLSL sources, quick start, sample configuration, canonical plan/report, offline Inspector, capture, license, and inner manifest. PDBs, loose shader binaries, separate capture/report bundles, and a duplicate manually uploaded source ZIP are excluded. Full verification, SHA-256 comparison, and two fresh-directory 240-frame WARP smokes are required before upload.
