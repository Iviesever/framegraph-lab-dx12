# Source-only release policy

Large binaries remain under ignored `artifacts/`. A Draft PR records relative path, size, SHA-256, generation command and validation result without attaching them. A future release requires separate authorization and contains an annotated tag, notes and GitHub automatic source archives only. It will not attach EXE, Win64 ZIP, PDB, compiled shaders, capture/report bundle, or a manually uploaded source archive.

This policy does not weaken local acceptance: Win64/source ZIP, primary capture/plan/report/Inspector, manifests/hashes and a fresh-directory 240-frame WARP smoke are mandatory before publication can be recommended. No tag/release/merge occurs in this goal.
