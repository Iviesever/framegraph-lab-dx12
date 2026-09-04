# Build authority and reproducibility

MQB is the primary local Windows C++/D3D12 build, incremental and run entry. CMake/CTest provide cross-platform Core, install/export and CI. Clang/GCC are supplementary compatibility and sanitizer evidence; failures remain blocking gates.

`build-manifest.json` is the sole list of C++ sources, target groups, project include directories, MSVC configuration/runtime/definitions/compile/link options and explicit system libraries. `tools/build_contract.py --write` generates strict `mqb.json` from that manifest. Normal builds run the same tool without `--write` and reject drift, missing/unlisted source files, or malformed/duplicate JSON keys. CMake consumes the manifest directly, then checks its actual target properties before completing configuration. No independently maintained CMake source list exists.

```powershell
./scripts/build.ps1 core -Configuration debug
./scripts/build.ps1 compiler_tests -Configuration debug -Run
./scripts/build.ps1 planner_tests -Configuration debug -Run
./scripts/build.ps1 property -Configuration release -Run --cases 10000
./scripts/build.ps1 fuzz -Configuration debug -Run --iterations 2000
./scripts/build.ps1 plan -Configuration release -Run
```

The small PowerShell boundary chooses a named target from the manifest and passes exact sources plus a real MQB profile to the installed tool. MQB owns C++ compilation/linking/archiving/cache and every `.mqb` output. MQB v5.4 supports one profile at a time; the generator expands configuration overlays rather than pretending profiles inherit. Default `mqb run --profile plan-debug` builds the compiler-plan tool. No generic MQB test verb or target-DAG feature is claimed. `scripts/build.ps1 app -Configuration debug -Run --warp --headless --frames 24` builds/runs the native executable. A project-local Python step generates truthful Git provenance; runtime shader compilation uses D3DCompiler.

Both local MSVC routes use x64, C++23, `/EHsc /W4 /permissive- /utf-8`, static CRT (`MTd` Debug / `MT` Release), Debug `/Od /Z7 /Ob0 /RTC1`, Release `/O2 /Ob2 /DNDEBUG`, and explicit equivalent linker policies from the manifest. CMake's machine environment is discovered by `scripts/with-msvc.ps1`; MQB uses its installed toolchain discovery. Local evidence verifies the same installed MSVC 14.51.36231 compiler path. MQB 5.4 rejects raw `/Zi` because compiler-PDB ownership is not represented; both routes therefore use supported `/Z7` embedded debug info. MQB emits D9025 diagnostics when its default W3/MD[d] flags precede the explicitly selected W4/MT[d] flags; effective last-option policy is recorded and this is not a source warning or a D3D12 message. MQB itself is not modified.

```powershell
./scripts/with-msvc.ps1 cmake --preset msvc-debug
./scripts/with-msvc.ps1 cmake --build --preset msvc-debug
ctest --preset msvc-debug
cmake --preset core-clang
cmake --build --preset core-clang
ctest --preset core-clang
cmake --install build/core-clang --prefix artifacts/install/core
```

Clang must be on the process PATH. This machine's portable toolchain is ignored under `.tools`; consumers may use their own installed Clang/GCC. No committed absolute toolchain path is required. Python 3 is required for the build-contract gate. `core-sanitized` enables ASan+UBSan. Local LLVM-MinGW rejects coverage-guided `-fsanitize=fuzzer`; bounded mutation smoke is verified and the libFuzzer entry can be used on a supported target.

The exported `Expected<T,E>` ABI is unconditionally project-owned. This prevents a compiler/standard-library mismatch such as Ubuntu Clang 18 with libstdc++ from changing public return type/layout across a built library and its consumer. The installed-package consumer test compiles and calls the public API after export.

Installing the Core exports `FrameGraphLab::framegraph_core` and public headers, with transitive C++23 requirements. UE is outside scope: MQB is not described as a replacement for UBT/UHT, and no other repository or global tool configuration is modified.
