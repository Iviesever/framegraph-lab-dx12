# Tier 2 measured results

Environment: Windows 11 x64, MSVC 19.51.36248 Release (`/O2`, static CRT), NVIDIA GeForce RTX 4070 Laptop GPU, driver 31.0.15.5161, D3D feature level 12.2, Debug Layer enabled. These are local observations with no cross-machine SLA.

## Core fixed-graph benchmark

Input: 64 transient UAV-capable buffers, 192 retained passes/usages, 31 raw samples, 1,000 operations per sample after warmup. The table uses raw JSON retained at `artifacts/benchmarks/core-before.json` and `artifacts/benchmarks/core-final.json`; `core-after.json` is the intermediate reserve iteration.

| Stage | Before median (µs) | After median (µs) | Change |
|---|---:|---:|---:|
| Graph compile | 52.5261 | 39.5762 | -24.65% |
| Allocation, alias on | 2.2198 | 2.3025 | +3.73% (untouched/noise) |
| Allocation, alias off | 1.9858 | 1.9322 | -2.70% (untouched/noise) |
| Barrier plan, alias on | 26.9801 | 25.3368 | -6.09% |
| Barrier plan, alias off | 26.5643 | 25.4990 | -4.01% |

The measured dominant stage was graph compile. Production changes only reserve vectors from already validated pass/resource/usage limits in compiler and barrier planning; allocation policy is unchanged. Before/after plan identity is `87a12f2fdda63447`. Both runs produce exactly 18,874,368 committed bytes, 2,359,296 alias-on physical bytes, 18,874,368 alias-off physical bytes, 56 reuse events, 192 transitions, 64 UAV barriers and 64 activation barriers. The allocation timing movement is not attributed to a code change.

The barrier audit removed nothing. For this graph, every transition is one of Common→UAV, UAV→ShaderRead or ShaderRead→Common needed for replay; each UAV barrier orders a write/read-write pair; each activation barrier implements the explicit placed-resource activation policy already checked by the independent oracle and hardware parity. There was no evidence supporting semantic removal.

## Native alias comparison

Raw summary: `artifacts/benchmarks/runtime-hardware-final.json`; 14 underlying reports are retained in its referenced `artifacts/benchmarks/runtime-raw/...` directory. The runner executed seven paired alias-on then alias-off runs serially from clean Tier 2 SHA `57dfbab8b328ecd644062f881e3a2bef96262584`, each at 1280x720, 240 frames, seed 24301, GPU indirect mode. Every pair had identical pixel hash, CPU/GPU visible count and Debug 0/0/0.

| Median | Alias on | Alias off | Observed difference |
|---|---:|---:|---:|
| Logical bytes | 16,589,456 | 16,589,456 | 0 |
| Reference/committed bytes | 17,825,792 | 17,825,792 | 0 |
| Actual heap bytes | 13,893,632 | 17,825,792 | -3,932,160 (-22.06%) |
| GPU graph work | 0.103273 ms | 0.104190 ms | -0.000917 ms |
| CPU graph record | 0.089782 ms | 0.091111 ms | -0.001329 ms |
| Full pass GPU total | 0.661856 ms | 0.659309 ms | +0.002547 ms |
| Full pass CPU record | 0.102736 ms | 0.104403 ms | -0.001668 ms |

The timing deltas are small local observations and do not justify changing the conservative alias activation policy. The physical-byte reduction is exact device heap evidence.

## Bounded stress

MQB/MSVC Release completed `framegraph_property --cases 100000`: 100,000 valid plus 100,000 invalid graphs passed independent dependency/topology/lifetime/allocation/transition/UAV/alias/identity checks. The same 100,000+100,000 sweep passed under Clang ASan+UBSan after CTest 12/12. `framegraph_fuzz --iterations 100000` passed its fixed-seed bounded mutation run. Logs: `tier2-property-100k.log`, `tier2-sanitized-property-100k.log` and `tier2-fuzz-100k.log`.
