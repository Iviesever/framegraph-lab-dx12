# Baseline toolchain observations

- `mqb run tests/unit/smoke.cpp --std 23 --debug --no-discover`: exit 0.
- First `cmake --preset msvc-debug`: exit 1, installed CMake does not recognize the VS 2026 generator. Selected Ninja Multi-Config and explicit process-scoped MSVC environment instead.
- First environment wrapper attempt: exit 1, native PowerShell/cmd quoting doubled the opening quote. Fixed the wrapper to invoke `call` with one quoted batch path. No compiler/source change.
- Repeated configure: exit 0, MSVC 19.51.36248.0.
- `cmake --build --preset msvc-debug`, `ctest --preset msvc-debug`: exit 0, 1/1 smoke passed.
- Existing unrelated GCC 4.9.2 cannot satisfy C++23; it is left untouched.
- LLVM-MinGW official release 20260826 downloaded and unpacked under ignored `.tools`; upstream documents ASan/UBSan support: https://github.com/mstorsjo/llvm-mingw . The archive SHA is recorded separately. No global installation or persistent PATH change.
- Repository is an ordinary standalone checkout (.git == common-dir), not a submodule. User's fixed new directory provides task isolation; no additional worktree created.
