# Fuzz execution scope

The bounded mutation harness exercises graph/descriptor/usage/ordering/memory validation and arbitrary JSON name bytes. It uses seed 0xD312F022 and runs 2,000 inputs in CTest. It passed under Clang 23 ASan+UBSan, plus MSVC and ordinary Clang. Full valid/invalid property sweeps provide structured successful paths that random invalid descriptions rarely reach.

An additional coverage-guided libFuzzer build was attempted. The installed LLVM-MinGW target rejects `-fsanitize=fuzzer` for x86_64-w64-windows-gnu despite shipping compiler-rt fuzzer archives; the recorded compiler command exited 1 before creating an executable. No local coverage-guided run is claimed. The portable `LLVMFuzzerTestOneInput` entry is available for a supported Clang target, and the bounded local smoke remains the currently verified fuzz evidence.
