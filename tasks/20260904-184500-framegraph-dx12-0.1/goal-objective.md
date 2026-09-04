# FrameGraphLab 0.1.0&#x20;

在 `D:\program\FrameGraphLab` 新建并完整交付公开仓库 `Iviesever/framegraph-lab-dx12`：一个现代 C++23 声明式 Render Graph 与 Direct3D 12 Transient Memory 实验室。

项目必须包含纯 C++、可跨平台测试的 Render Graph Compiler，以及真实 Win32 / Direct3D 12 执行后端。后端必须实际执行自动资源状态转换、Transient Placed Resource 内存复用、Aliasing Barrier、GPU 时间戳和多个渲染/后处理 Pass，并交付一个视觉清晰的程序化 3D Demo、无窗口 WARP Smoke、PNG 截图、Canonical Graph Plan、交互式 HTML Inspector、性能报告、Win64 ZIP、完整测试证据和 Draft PR。

不要创建多个仓库，不要修改 SeedForge、RollbackLab、MQB 或其他项目。持续自主工作，直到所有 P0 验收条件通过、功能分支已推送并创建 Draft PR；不要在只完成规划、只画出三角形、只通过编译、只完成纯 Graph Compiler 或只创建 PR 时提前结束。

## 一、目录、Git 与权限边界

1. 工作目录固定为：
```text
D:\program\FrameGraphLab
```

2. 开始时检查目录：
   - 不存在则创建；
   - 已存在且非空时先识别内容；
   - 不覆盖任何来源不明的现有文件。
3. 初始化 Git 仓库，默认分支为 `main`。
4. 创建最小基线：
   - `README.md`
   - `LICENSE`：MIT
   - `.gitignore`
   - `.gitattributes`
   - `AGENTS.md`
   - `CMakeLists.txt`
   - `CMakePresets.json`
5. 使用当前已认证的 GitHub 方式创建公开仓库：
```text
Iviesever/framegraph-lab-dx12
```

建议描述：
```text
Modern C++23 render-graph compiler with Direct3D 12 transient-resource aliasing, automatic barriers, GPU timing, deterministic plans, and a procedural post-processing demo.
```

6. 首先把最小基线提交到 `main`，推送后再从真实最新 `origin/main` 创建：
```text
feat/framegraph-dx12-0.1
```

7. 所有源码、测试、临时文件、截图、报告、Shader 和构建产物必须位于 `D:\program\FrameGraphLab`。
8. 禁止修改：
   - `D:\program\SeedForge`
   - `D:\program\RollbackLab`
   - MQB、本地 UE Engine 或其他仓库。
9. 当前可能仍有 SeedForge 审计进程。不得停止或杀死来源不明的 UnrealEditor、UnrealBuildTool、AutomationTool、Cook 或 Package 进程。
10. 发现其他项目正在执行重型 UE/GPU 验证时：
    - 继续 Render Graph Core、测试、文档和 CPU Property Sweep；
    - 暂缓长时间 Hardware/WARP Capture 或 GPU Benchmark；
    - 不通过杀进程夺取资源。
11. 本 Goal 授权：
    - 创建仓库；
    - 创建和修改功能分支；
    - 构建、运行、测试和截图；
    - 使用 Windows SDK 自带组件；
    - Commit；
    - Push；
    - 创建或更新 Draft PR。
12. 未授权：
    - 合并 PR；
    - 删除远端分支；
    - 创建或移动 Tag；
    - 发布正式 GitHub Release；
    - 修改系统级配置；
    - 安装全局长期服务；
    - 写入凭据或个人信息。
13. 使用一个主 Agent，最多两个互不重叠的子 Agent：
    - Graph Compiler / Property Review；
    - D3D12 / GPU Validation Review。
14. 子 Agent 优先只读审查、测试设计和日志分析。不得同时写同一文件。
15. 同时只运行一个 FrameGraphLab GPU Demo、Capture、WARP Stress 或 Benchmark 进程。

## 二、持久进度与完全重置检查点

在实现生产代码前创建：
```text
tasks/<timestamp>-framegraph-dx12-0.1/
├─ product_contract.md
├─ architecture.md
├─ implementation_plan.md
├─ verification_matrix.md
├─ progress.md
├─ reset_handoff.md
└─ evidence/
```

每个 PACT 完成后必须：

1. 更新 `progress.md`；
2. 记录 exact HEAD；
3. 记录运行命令和退出码；
4. 记录测试、Debug Layer、截图和产物状态；
5. 创建独立 Commit；
6. Push 当前分支；
7. 保持可恢复的 last-known-good revision。

如果用户执行 `/goal pause`，则：

1. 不开始新的结构性修改；
2. 完成当前可以安全结束的原子操作；
3. 停止长时间进程；
4. 写入 `reset_handoff.md`：
   - 当前 PACT；
   - exact HEAD；
   - 工作树状态；
   - 已完成合同；
   - 已知 Blocker；
   - 下一条命令；
5. 尽可能保持 clean working tree；
6. Push 检查点后暂停。

用户应用 Banked Reset 并输入 `/goal resume` 后：

1. 重新读取 `AGENTS.md`、`product_contract.md`、`progress.md` 和 `reset_handoff.md`；
2. Fresh fetch；
3. 核对本地/远端 exact HEAD；
4. 不重复已经完成的工作；
5. 从下一未完成验收合同继续。

不要因额度重置新开第二个项目或第二个仓库。

## 三、唯一产品目标

最终数据流必须是：
```text
Declarative Passes + Resource Usages
                  │
                  ▼
        Pure C++ Graph Compiler
                  │
        ┌─────────┼─────────┐
        ▼         ▼         ▼
 Dependencies  Lifetimes  Resource States
        │         │         │
        └─────────┼─────────┘
                  ▼
      Stable Compiled Graph Plan
                  │
        ┌─────────┼─────────┐
        ▼         ▼         ▼
 Topological   Transient   Barrier
   Schedule    Allocation    Plan
                  │
                  ▼
       Direct3D 12 Executor
                  │
        Placed Resources / Heaps
                  │
                  ▼
 Depth → HDR Scene → Bloom → Tone Map → Present
                  │
                  ▼
 Screenshot + Timing + Graph Inspector + Evidence
```

P0 必须证明：

1. Graph Compiler 不依赖 Direct3D、Windows、GPU 或 Wall Clock。
2. 相同 Graph 输入生成字节稳定的 Compiled Plan。
3. D3D12 后端消费同一个 Compiled Plan，不维护第二套依赖或 Barrier 决策。
4. 实际运行使用 Placed Resources 和至少一次真实内存 Aliasing。
5. Aliasing 开启和关闭时，最终同一帧 RGBA Readback 完全一致。
6. D3D12 Debug Layer 不报告未分类的 Error/Corruption。
7. Hardware Adapter 与 WARP 都有明确选择、回退和报告。
8. 可执行 Demo 可以正常交互，也可以无窗口自动运行并退出。
9. 所有验证和 Artifact 绑定同一个 clean Git HEAD。

## 四、PACT-00：产品合同与最小基线

先建立：
```text
include/framegraph/**
src/core/**
src/d3d12/**
src/app/**
shaders/**
tests/unit/**
tests/property/**
tests/d3d12/**
tools/**
viewer/**
docs/**
scripts/**
tasks/**
```

基线要求：

- C++23；
- CMake 3.28+；
- MSVC x64 Debug/Release Presets；
- Clang 或 GCC Core-only Preset；
- CTest；
- Windows SDK；
- Direct3D 12、DXGI、D3DCompiler、WIC；
- 不使用大型第三方 Framework；
- 不复制 Microsoft Sample 作为主体实现；
- Windows 后端可以使用 `Microsoft::WRL::ComPtr`；
- Core 不包含 `Windows.h`、D3D12 或 COM 类型。
- 如果mqb构建工具能成功构建ue项目，优先用mqb

锁定：

- P0、P1、P2；
- 依赖方向；
- Graph 术语；
- Barrier 语义；
- Whole-resource v0.1 边界；
- Transient Memory 语义；
- Debug Layer Policy；
- Capture Policy；
- Non-goals；
- Validation Matrix。

完成最小 MSVC/Clang Build 和 CTest Smoke 后提交。

## 五、PACT-10：纯 C++ Render Graph Compiler

实现清晰、强类型的核心 API，名称可以调整，但职责必须等价：
```text
GraphBuilder
GraphCompiler
GraphDescription
CompiledGraph
PassId
ResourceId
TextureDesc
BufferDesc
ResourceUsage
ResourceAccess
ResourceLifetime
DependencyEdge
CompiledPass
GraphError
```

### 资源和 Pass

支持：

- Transient Texture；
- Transient Buffer；
- Imported Texture；
- Imported Buffer；
- Exported Output；
- Side-effect Pass；
- Read；
- Write；
- ReadWrite/UAV；
- Render Target；
- Depth Read/Write；
- Shader Resource；
- Copy Source/Destination；
- Present。

v0.1 可以只支持 Whole-resource State，不实现复杂 Subresource Range；必须准确记录限制，不允许假装已有 Subresource Tracking。

### Graph 编译

必须实现并测试：

1. Resource/Pass Handle 验证；
2. Descriptor 验证；
3. Read-after-write；
4. Write-after-read；
5. Write-after-write；
6. Stable Dependency Edge；
7. Stable Topological Sort；
8. Cycle Detection；
9. Cycle Diagnostic，至少给出相关 Pass 链；
10. 从 Exported/Present/Side-effect Root 反向执行 Dead Pass Elimination；
11. First Use / Last Use；
12. Imported Resource Initial State；
13. Imported Resource Required Final State；
14. 未初始化读取；
15. 重复或冲突声明；
16. 无生产者的 Transient Read；
17. 无界数量和整数溢出保护；
18. Typed Errors，而不是只依赖字符串；
19. Canonical Plan Serialization；
20. Stable Plan Identity Hash。

不能依赖：

- `unordered_map`/`unordered_set` 的迭代顺序；
- 指针地址；
- Wall Clock；
- 随机设备；
- 文件系统顺序。

每个语义先写 RED test，再完成最小 GREEN。

完成后运行 Unit 与 Property Smoke 并提交。

## 六、PACT-20：Transient Allocation 与 Barrier Planner

实现纯 C++、不依赖 D3D12 类型的规划层：
```text
TransientAllocator
MemoryRequirement
HeapClass
PhysicalAllocation
AliasingEvent
ResourceStatePlan
TransitionBarrierPlan
UavBarrierPlan
AliasingBarrierPlan
```

### Transient Memory Planner

要求：

1. 输入：
   - Resource Lifetime；
   - Size；
   - Alignment；
   - Heap Class；
   - Compatibility；
   - Dedicated Allocation Requirement。
2. 输出：
   - Physical Heap；
   - Offset；
   - Size；
   - Alignment；
   - Reused Region；
   - Aliasing predecessor/successor。
3. 只有生命周期不重叠且 Heap Class 兼容的资源可以共享物理区域。
4. 所有 Offset 必须满足 Alignment。
5. 不允许区间重叠导致同时存活资源共享字节。
6. 规划顺序稳定。
7. 提供：
   - Aliasing Enabled；
   - Aliasing Disabled/reference；
   - Committed Bytes；
   - Physical Heap Bytes；
   - Saved Bytes；
   - Savings Ratio。
8. 极端 Size/Alignment 必须检查溢出。
9. Planner 不以“最优装箱”作虚假声明；使用明确、稳定、可解释的算法。
10. Property Test 必须独立验证每个活跃区间，而不是复用生产 Planner 的判断。

### Barrier Planner

要求：

1. 从 Compiled Pass Usage 推导资源状态；
2. 为 Imported Resources 使用显式 Initial State；
3. 为每个 Pass 生成必要 Transition；
4. 连续相同状态不生成冗余 Transition；
5. UAV Write Ordering 需要明确 UAV Barrier 语义；
6. Physical Region 从一个 Placed Resource 转移给另一个时生成 Aliasing Event；
7. Imported Backbuffer 最终恢复 Present；
8. 非法或不支持的 Usage Combination fail closed；
9. Barrier Plan 必须稳定并进入 Plan Identity；
10. D3D12 后端只做枚举映射与命令记录，不重新判断依赖关系。

### Property/Fuzz

至少执行：

- 10,000 个有效随机 DAG；
- 10,000 个包含 Cycle、未初始化读取或非法 Usage 的无效 Graph；
- 重复运行 Identity 一致；
- 所有有效 Graph：
  - 拓扑满足边；
  - 生命周期正确；
  - 活跃资源不非法重叠；
  - Transition 前后连续；
  - Alias Event 覆盖所有物理复用；
- 无崩溃、无未定义行为、无无界运行。

完成后提交并形成适合 Banked Reset 前恢复的稳定检查点。

## 七、PACT-30：Direct3D 12 Runtime 基础

实现原生 Win32 / Direct3D 12 Runtime：
```text
Dx12Context
AdapterSelector
DeviceContext
SwapChain
FrameContext
CommandQueue
DescriptorAllocator
FenceTimeline
ShaderCompiler
ResourceFactory
ReadbackCapture
DebugMessageCollector
```

要求：

1. Debug 构建在创建设备前启用 D3D12 Debug Layer。
2. 支持：
   - 自动选择 Hardware Adapter；
   - `--hardware` 强制硬件；
   - `--warp` 强制 WARP；
   - 不支持时给出 typed failure。
3. 三帧或明确数量的 Frames in Flight。
4. 每帧独立 Command Allocator 和 Fence Value。
5. 正确等待资源复用，不执行每帧全 GPU Flush。
6. Swapchain Resize：
   - 等待必要 Fence；
   - 释放旧 Backbuffer；
   - ResizeBuffers；
   - 重建 RTV；
   - 重新编译依赖尺寸的 Graph；
   - 不泄漏 Descriptor/Resource。
7. 支持最小化窗口、零尺寸和恢复。
8. Device Removed 时记录：
   - HRESULT；
   - `GetDeviceRemovedReason`；
   - 当前 Graph/Pass；
   - 可用诊断。
9. Shader 编译错误必须保留文件、入口、Target 和 Compiler Diagnostic。
10. 不依赖机器专有绝对路径。
11. 所有 COM、HANDLE、Window 和资源使用 RAII。
12. 无 detached thread。
13. 无无限等待；所有自动验证有 Watchdog。
14. CLI 至少支持：
```text
framegraph_lab --hardware
framegraph_lab --warp
framegraph_lab --headless
framegraph_lab --frames 240
framegraph_lab --scene-seed 24301
framegraph_lab --capture <png>
framegraph_lab --report <json>
framegraph_lab --plan <json>
framegraph_lab --aliasing on|off
```

先完成 Clear/Present Smoke，再完成隐藏窗口 WARP Smoke。不要把“能清屏”描述为项目完成。

## 八、PACT-40：真实 Graph Executor 与 Placed Resource Aliasing

实现：
```text
Dx12GraphExecutor
Dx12PlacedResourceArena
Dx12BarrierEmitter
Dx12PassContext
Dx12DescriptorBindings
Dx12TimestampQueries
```

要求：

1. Executor 直接消费 `CompiledGraph`。
2. Pass Callback 只能访问该 Pass 已声明的资源。
3. Debug 构建发现未声明访问时 fail closed。
4. Transient Resources 使用：
   - `ID3D12Heap`；
   - `CreatePlacedResource`；
   - Planner 给出的 Offset；
   - Planner 给出的 Heap Class。
5. 当同一物理区域从 Resource A 转移到 Resource B 时发出正确 Aliasing Barrier。
6. 新激活的 Render Target/Depth Resource 必须按实际 D3D12 规则完成必要初始化。
7. Transition/UAV/Aliasing Barrier 来自 Core Plan。
8. Imported Swapchain Backbuffer 不进入 Transient Heap。
9. 资源销毁必须服从 Fence Lifetime。
10. Graph 只在：
    - Resize；
    - Scene/Feature Configuration 变化；
    - Aliasing Policy 变化时重新编译；
      正常每帧不重复进行完整 Graph 编译。
11. 提供 Debug 开关：
    - Aliasing On；
    - Aliasing Off；
    - Barrier Trace；
    - Resource Lifetime Trace。
12. 同一 Scene Seed、Camera、Frame 下：
    - Aliasing On RGBA Readback；
    - Aliasing Off RGBA Readback；
      必须逐字节相同。
13. 报告真实：
    - Logical Resource Bytes；
    - Physical Heap Bytes；
    - Saved Bytes；
    - Heap Count；
    - Transition Count；
    - UAV Barrier Count；
    - Aliasing Barrier Count。
14. 不把 Planner 预测值冒充实际 D3D12 Heap Allocation；二者都要记录并核对。

## 九、PACT-50：程序化渲染 Demo

不使用外部美术资产，创建具有清晰视觉效果的程序化场景。

建议场景：
```text
Neon Ruins
- Instanced cubes/pillars
- Procedural floor grid
- Rotating/orbit camera
- Moving key light
- Emissive objects
- High-contrast HDR palette
```

必须至少包含以下真实 Pass：
```text
DepthPrepass
SceneHDR
BloomExtract
BloomBlurHorizontal
BloomBlurVertical
ToneMap
Present
```

可以根据真实实现调整，但必须：

1. 至少一个 Depth Attachment；
2. 至少一个 HDR Render Target；
3. 至少两个可形成真实 Aliasing Opportunity 的 Transient Resource；
4. 至少一个 UAV/Compute 或明确写后读同步场景；
5. Fullscreen Composite；
6. 最终写入 Swapchain；
7. GPU Timestamp 覆盖每个主要 Pass。

场景要求：

- 16:9；
- 默认 1280×720；
- 视觉结果非空、非纯色；
- 清楚展示 Bloom、深度和几何层次；
- 不需要 PBR 材质系统；
- 不需要外部 Texture；
- 不需要骨骼、动画或关卡编辑器；
- Input 支持：
  - 鼠标或键盘旋转相机；
  - Pause；
  - Single Step；
  - Reset；
  - Aliasing On/Off；
  - Debug Resource View；
  - Barrier/Pass Overlay 可以使用窗口标题或简洁内置绘制。

自动 Capture 必须：

1. 固定 Scene Seed；
2. 固定 Camera；
3. 固定 Logical Frame；
4. 读取最终 RGBA；
5. 使用 WIC 或等价系统组件写 PNG；
6. 验证尺寸；
7. 验证非黑像素比例；
8. 验证亮度/颜色分布；
9. 记录 Pixel Hash；
10. 不把跨 GPU 像素一致性作为普遍保证。

## 十、PACT-60：Graph Inspector、报告与性能证据

生成：
```text
artifacts/reports/framegraph-plan.json
artifacts/reports/frame-report.json
artifacts/viewer/framegraph-inspector.html
artifacts/captures/neon-ruins.png
```

### Canonical Plan JSON

至少包含：

- Git SHA；
- Plan Schema Version；
- Scene Seed；
- Active/Culled Passes；
- Stable Pass Order；
- Dependency Edges；
- Logical Resources；
- First/Last Use；
- Physical Heap/Offset；
- Alias Chains；
- Transition/UAV/Aliasing Barriers；
- Plan Identity。

时间信息不得进入 Plan Identity。

### Runtime Report

至少包含：

- Adapter；
- Vendor/Device；
- Hardware/WARP；
- Driver 可获取信息；
- D3D Feature Level；
- Resolution；
- Frames in Flight；
- Logical/Physical Bytes；
- Alias Savings；
- Pass CPU Record Time；
- Pass GPU Time；
- Barrier Counts；
- Pixel Hash；
- Debug Layer Error/Warning Counts；
- Device Removed Result；
- Success/Failure；
- Git SHA。

GPU 时间仅是本机观察，不作跨机器 SLA。

### HTML Inspector

必须是自包含静态 HTML，不依赖 CDN、Node Runtime 或服务器，展示：

- Pass DAG；
- Stable Execution Order；
- Resource Lifetime Bars；
- Physical Heap Timeline；
- Alias Region；
- Transition/UAV/Aliasing Barrier；
- Culled Pass；
- 每 Pass GPU Time；
- Logical versus Physical Memory；
- Plan Identity；
- Screenshot Preview。

使用真实 Plan/Report，禁止硬编码虚假演示数据。

实际在浏览器中检查：

- 无 Console Error；
- Desktop 视口可用；
- Narrow 视口无严重溢出；
- Pass 选择与 Resource Highlight 可用；
- Lifetime/Alias 信息与 JSON 一致。

提交小型 Sample Plan/Report 和 Inspector；大型运行产物放入 ignored `artifacts/`。

## 十一、PACT-70：条件冲刺——GPU-Driven Culling

只有同时满足：
```text
全部 P0 已绿色
Packaged Demo 已经成功
当前时间早于 2026-09-05 09:30 JST
重置后额度仍高于约 25%
```

才允许开始。

实现：

- GPU Frustum Culling Compute Pass；
- Visible Instance Buffer；
- Indirect Argument Buffer；
- `ExecuteIndirect`；
- CPU Reference Culling；
- 固定 Camera 下 CPU/GPU Visible Count 一致；
- Buffer State/UAV Barrier 由同一 Graph Plan 管理；
- Report 展示 Input/Visible Instance Count；
- 可开关 CPU Direct Draw 与 GPU Indirect Draw；
- 输出相同场景语义。

禁止在该冲刺中加入：

- Async Compute Queue；
- Occlusion Hierarchy；
- Mesh Shader；
- Ray Tracing；
- Bindless Framework；
- Vulkan。

如果同一问题两次修补仍未闭环：

1. 写 Root Cause Packet；
2. 回滚不完整的 PACT-70 生产代码；
3. 保留 P0；
4. 在 Known Limitations 中记录；
5. 不把它描述为已完成。

## 十二、PACT-80：全量验证、作品集与独立审计

### Core 验证

至少执行：

- MSVC Debug；
- MSVC Release；
- ASan；
- UBSan；
- Unit Tests；
- 10,000 Valid Graph Sweep；
- 10,000 Invalid Graph Sweep；
- Canonical Plan Repeat；
- Allocation Invariant；
- Barrier Invariant；
- Fuzz Smoke；
- Clean Rebuild。

### D3D12 验证

至少执行：

- Hardware interactive run；
- WARP hidden-window run；
- Debug Layer run；
- 1,000-frame WARP stress；
- 1,000-frame Hardware stress，在硬件可用时；
- Resize stress；
- Minimize/restore；
- Aliasing On；
- Aliasing Off；
- On/Off RGBA parity；
- Shader failure negative；
- Unsupported adapter negative；
- Invalid graph negative；
- Capture timeout negative；
- Device/report parsing；
- 无残留进程。

D3D12 Debug Layer：

- Error：0；
- Corruption：0；
- 未分类 Warning：0；
- 必要、已解释的系统信息可以做最小精确 Allow-list；
- 禁止整体关闭 Debug Layer 或静音未知消息。

### 打包

生成：
```text
artifacts/release/FrameGraphLab-Win64-0.1.0-<sha>.zip
artifacts/release/FrameGraphLab-Source-0.1.0-<sha>.zip
artifacts/release/DELIVERY_MANIFEST.json
artifacts/release/*.sha256
```

Win64 ZIP 至少包含：

- `FrameGraphLab.exe`；
- 必要 Shader；
- License；
- Quick Start；
- Sample Config；
- Sample Plan/Report；
- HTML Inspector；
- Screenshot；
- Manifest。

在全新临时目录解压并运行：
```text
FrameGraphLab.exe --warp --headless --frames 240 --capture smoke.png --report smoke.json --plan plan.json
```

验证：

- Exit 0；
- PNG 合法；
- JSON 可解析；
- Pixel 非空；
- Plan Identity 存在；
- 至少一次真实 Aliasing；
- Debug Layer 无 Error/Corruption；
- 所有路径位于解压目录；
- 不依赖开发树的隐藏 DLL/Shader。

### GitHub CI

至少：

- Windows MSVC Core；
- Ubuntu Clang/GCC Core；
- Unit/Property Smoke；
- Canonical Plan；
- Build；
- 文档链接。

本机 Windows + D3D12 验证是图形交付权威。GitHub Hosted Runner 不具备可靠 GPU 时，不虚假声称完成硬件渲染验证。

### 文档

至少创建：
```text
README.md
docs/ARCHITECTURE.md
docs/RENDER_GRAPH_COMPILER.md
docs/DEPENDENCY_AND_CULLING.md
docs/TRANSIENT_MEMORY.md
docs/RESOURCE_BARRIERS.md
docs/D3D12_BACKEND.md
docs/CAPTURE_AND_REPORTS.md
docs/BENCHMARKING.md
docs/TESTING.md
docs/CODE_WALKTHROUGH.md
docs/INTERVIEW_GUIDE.md
docs/LIVE_CHANGE_DRILLS.md
docs/KNOWN_LIMITATIONS.md
docs/AI_ASSISTANCE.md
docs/RELEASE_NOTES_0.1_CANDIDATE.md
```

README 首屏必须在 30 秒内说明：
```text
Declarative Passes
        ↓
Dependency + Lifetime Compile
        ↓
Transient Heap Aliasing
        ↓
Automatic D3D12 Barriers
        ↓
HDR / Bloom / Tone Map
        ↓
GPU Timing + Inspector + Packaged Evidence
```

并包含：

- 真实截图；
- Graph Inspector 截图；
- 最短构建命令；
- 最短 WARP Smoke 命令；
- Hardware Demo 命令；
- 测试数量；
- Property/Fuzz 结果；
- Memory Savings；
- Debug Layer 结果；
- Known Limitations；
- AI Assistance。

Interview Guide 至少回答：

- Render Graph 解决什么问题；
- RAW、WAR、WAW 的区别；
- 为什么需要 Stable Topological Sort；
- Dead Pass 如何确定；
- Resource Lifetime 如何定义；
- 为什么生命周期不重叠才可 Alias；
- Logical Resource 与 Physical Allocation 的区别；
- Committed Resource 与 Placed Resource 的区别；
- Transition、UAV、Aliasing Barrier 各自解决什么问题；
- 为什么 Backbuffer 是 Imported Resource；
- 为什么 Graph 不应该每帧重新完整编译；
- Fence 和 Frames in Flight 如何防止过早复用；
- WARP 能证明什么、不能证明什么；
- Debug Layer 能证明什么、不能证明什么；
- GPU Timestamp 为什么不进入 Deterministic Plan Identity；
- 为什么跨 GPU 不承诺 Pixel Hash 相同；
- 为什么没有实现 Async Compute；
- 如何把这个 Graph 接入 UE/RHI，但本项目为何保持独立；
- AI 完成了什么，用户能诚实声明什么。

Live Change Drills 至少包含 12 个由易到难的练习。

### AI 署名

准确记录：

- 用户定义职业目标、产品方向、范围、截止时间、额度策略和验收标准；
- Codex GPT-5.6 Sol 负责架构细化、代码、测试、D3D12 调试、截图、打包、审计和文档；
- 用户本轮不参与手写交付代码；
- 项目必须描述为 AI-assisted engineering；
- 不得声称用户独立手写；
- 面试展示前用户必须亲自完成至少一个 Live Change Drill，并能解释 Graph Compile、Resource Lifetime、Aliasing、Barrier、Fence 和 WARP。

### 独立审计

使用新的只读审查上下文，完整审查 `origin/main...HEAD`，至少覆盖：

- Dependency Correctness；
- Stable Ordering；
- Cycle/Culling；
- Lifetime；
- Alignment/Overflow；
- Alias Safety；
- Barrier Continuity；
- Imported Final State；
- D3D12 Object Lifetime；
- Fence；
- Descriptor Lifetime；
- Resize；
- Device Removed；
- Shader Errors；
- Debug Layer；
- Capture Freshness；
- Artifact Freshness；
- 文档与 AI 署名。

只修复确认存在的 Blocker/High，以及低风险、直接相关的 Medium。最终审计阶段不增加新图形功能。

## 十三、严格非目标

本轮禁止：

- 创建第二个新仓库；
- 修改 SeedForge、RollbackLab、MQB；
- Unreal 或 Unity 集成；
- Vulkan、OpenGL 或 Metal 后端；
- Ray Tracing；
- Mesh Shader；
- 完整 PBR；
- glTF/FBX 资产管线；
- 大型 Texture 系统；
- 骨骼动画；
- ECS；
- 完整游戏引擎；
- Editor；
- ImGui 大型依赖；
- RenderDoc 自动化硬依赖；
- Async Compute；
- 多 GPU；
- 全面 Subresource Tracking；
- 生产级 Shader 热重载；
- 网络功能；
- 云服务；
- 为截图牺牲 Barrier 正确性；
- 用 Committed Resource 冒充 Placed Resource Aliasing；
- 只计算理论 Savings 却声称实际显存复用；
- 关闭 Debug Layer 掩盖问题；
- 用大量 Sleep 或无限重试掩盖 Fence/Capture 竞态；
- 为耗额度继续加入无关功能。

## 十四、修复纪律

每个行为变更：
```text
Acceptance Contract
→ RED Test
→ Minimal GREEN
→ Focused Regression
→ Relevant Full Regression
→ Evidence
→ Scoped Commit
```

同一故障两次修补仍失败时创建：
```text
tasks/.../evidence/root-cause-<issue>.md
```

记录：

- Observable Symptom；
- Minimal Reproduction；
- Debug Layer Messages；
- DRED/Device Removed 信息；
- 已确认事实；
- 已排除假设；
- 真正所有权边界；
- 下一种修复为何不同。

禁止第三次无证据猜测。

## 十五、时间与功能冻结

外部截止：
```text
2026-09-05 15:30 UTC+8
2026-09-05 16:30 JST
```

内部硬停止：
```text
2026-09-05 15:00 UTC+8
2026-09-05 16:00 JST
```

功能冻结：
```text
2026-09-05 11:30 UTC+8
2026-09-05 12:30 JST
```

重置建议边界：
```text
当前用量降到约 2%～4%
或 2026-09-05 01:00 JST
以先发生者为准
```

功能冻结后只允许：

- 修 Blocker；
- Core Tests；
- Property/Fuzz；
- WARP/Hardware Stress；
- Debug Layer；
- Resize/Capture；
- Screenshot；
- Benchmark；
- Package；
- Manifest/SHA；
- 文档事实校准；
- 独立审计；
- Commit/Push/Draft PR。

功能冻结后禁止添加 PACT-70 或任何新 Pass。

## 十六、最终完成条件

只有同时满足以下条件，才能报告完成：

1. 公开仓库已创建；
2. `main` 基线存在；
3. 功能分支基于真实最新 `main`；
4. Core 不依赖 Windows/D3D12；
5. Graph 输入生成稳定 Plan；
6. RAW/WAR/WAW 正确；
7. Stable Topological Sort 正确；
8. Cycle Detection 正确；
9. Dead Pass Elimination 正确；
10. Lifetime 正确；
11. Allocation Alignment 正确；
12. 同时存活资源不非法共享物理区域；
13. Barrier Plan 连续正确；
14. Imported Backbuffer 最终回到 Present；
15. 10,000 Valid Graph Sweep 通过；
16. 10,000 Invalid Graph Sweep 通过；
17. MSVC Debug/Release 通过；
18. Clang/GCC Core 通过；
19. ASan/UBSan 通过；
20. Hardware Demo 启动；
21. WARP Headless Smoke 通过；
22. 实际使用 `CreatePlacedResource`；
23. 至少一次实际 Aliasing；
24. Aliasing Barrier 已执行；
25. Aliasing On/Off RGBA Readback 一致；
26. D3D12 Error/Corruption 为 0；
27. HDR/Bloom/ToneMap 场景真实执行；
28. GPU Timestamp 报告有效；
29. PNG Capture 合法且非空；
30. Canonical Plan/Runtime Report 可解析；
31. HTML Inspector 使用真实报告并经过浏览器检查；
32. Win64 ZIP 从全新目录运行通过；
33. Manifest 和 SHA 绑定同一 clean HEAD；
34. README/Architecture/Limitations 与代码一致；
35. AI Assistance 准确；
36. 工作树 clean；
37. 分支已 Push；
38. Draft PR 已创建；
39. PR Body 包含：
    - 产品目标；
    - Graph Architecture；
    - Dependency/Culling；
    - Transient Allocation；
    - Barrier Rules；
    - D3D12 Backend；
    - exact base/head；
    - test matrix；
    - property/fuzz；
    - WARP/hardware；
    - Debug Layer；
    - alias savings；
    - pixel parity；
    - benchmark；
    - screenshots；
    - artifacts/SHA；
    - Known Limitations；
    - PACT-70 完成或跳过状态；
    - AI Assistance。
40. 不合并 PR；
41. 不创建 Tag；
42. 不发布正式 Release。

## 十七、最终报告格式

最终必须报告：
```text
Repository
Branch
Base SHA
Final HEAD
Ahead/behind
Commit list
Core source/module overview
Graph plan schema/version
Pass count
Logical resource count
Culled pass count
Dependency edge count
Transition/UAV/Aliasing barrier counts
Logical bytes
Physical heap bytes
Alias saved bytes/ratio
Unit test totals
Valid/invalid property sweep totals
Fuzz result
MSVC result
Clang/GCC result
ASan/UBSan result
Hardware adapter result
WARP result
Debug Layer Error/Warning/Corruption
Resize stress result
Aliasing On/Off RGBA parity
Pixel hash
GPU timing observations
PACT-70 status
Win64 artifact path/SHA
Source artifact path/SHA
Plan/report/viewer/screenshot paths
Draft PR number
Independent audit findings
Known limitations
AI authorship statement
Whether merge is recommended
Whether v0.1.0 publication is recommended
User’s shortest next authorization
```

持续自主工作，不要在只完成规划、只完成 Graph Compiler、只渲染一个三角形、只在 Debug 中运行或只创建 Draft PR 时提前结束。