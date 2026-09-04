param([switch]$SkipHardware)
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot));Set-Location -LiteralPath $root
$env:TEMP=Join-Path $root 'artifacts/tmp';$env:TMP=$env:TEMP;New-Item -ItemType Directory -Path $env:TEMP -Force|Out-Null
if(git status --porcelain){throw 'Full verification requires clean working tree'}
$head=(git rev-parse HEAD).Trim();$out=Join-Path $root "artifacts/verification/$head";New-Item -ItemType Directory -Path $out -Force|Out-Null
$steps=[Collections.Generic.List[object]]::new()
function Step([string]$Name,[scriptblock]$Body){
  $watch=[Diagnostics.Stopwatch]::StartNew();$log=Join-Path $out ($Name+'.log')
  try{& $Body *>&1|Tee-Object -FilePath $log;if(-not $?){throw "$Name failed"};$steps.Add([ordered]@{name=$Name;status='passed';elapsed_ms=$watch.ElapsedMilliseconds;log=[IO.Path]::GetRelativePath($root,$log)})}
  catch{$steps.Add([ordered]@{name=$Name;status='failed';elapsed_ms=$watch.ElapsedMilliseconds;error=$_.Exception.Message;log=[IO.Path]::GetRelativePath($root,$log)});throw}
}
try{
  Step docs {python tools/check_docs.py;if($LASTEXITCODE){throw 'docs'}}
  Step mqb-unit {foreach($target in @('compiler_tests','planner_tests','boundary_tests','options_tests','shader_tests')){./scripts/build.ps1 $target -Configuration debug -Run;if($LASTEXITCODE){throw $target}}}
  Step mqb-benchmark {./scripts/build.ps1 benchmark -Configuration release;if($LASTEXITCODE){throw 'benchmark build'};& './.mqb/bin/framegraph_benchmark-release.exe' --samples 31 --iterations 1000 | Set-Content (Join-Path $out 'core-benchmark.json') -Encoding utf8;if($LASTEXITCODE){throw 'benchmark run'};python tests/property/validate_benchmark.py .mqb/bin/framegraph_benchmark-release.exe;if($LASTEXITCODE){throw 'benchmark schema'}}
  Step mqb-property {./scripts/build.ps1 property -Configuration release -Run --cases 100000;if($LASTEXITCODE){throw 'property'}}
  Step mqb-fuzz {./scripts/build.ps1 fuzz -Configuration release -Run --iterations 100000;if($LASTEXITCODE){throw 'fuzz'}}
  Step mqb-app {./scripts/build.ps1 app -Configuration release;if($LASTEXITCODE){throw 'app'}}
  Step msvc-debug {./scripts/with-msvc.ps1 cmake --preset msvc-debug --fresh;if($LASTEXITCODE){throw 'configure'};./scripts/with-msvc.ps1 cmake --build --preset msvc-debug;if($LASTEXITCODE){throw 'build'};ctest --preset msvc-debug;if($LASTEXITCODE){throw 'ctest'}}
  Step msvc-release {./scripts/with-msvc.ps1 cmake --build --preset msvc-release;if($LASTEXITCODE){throw 'build'};ctest --preset msvc-release;if($LASTEXITCODE){throw 'ctest'}}
  $env:PATH=(Join-Path $root '.tools/llvm-mingw-20260826-ucrt-x86_64/bin')+';'+$env:PATH
  Step clang {cmake --preset core-clang --fresh;if($LASTEXITCODE){throw 'configure'};cmake --build --preset core-clang;if($LASTEXITCODE){throw 'build'};ctest --preset core-clang;if($LASTEXITCODE){throw 'ctest'}}
  Step cmake-install {
    $prefix=Join-Path $out 'install';cmake --install build/core-clang --prefix $prefix;if($LASTEXITCODE){throw 'install'}
    $consumer=Join-Path $out 'consumer';New-Item -ItemType Directory -Path $consumer -Force|Out-Null
    [IO.File]::WriteAllText((Join-Path $consumer 'CMakeLists.txt'),"cmake_minimum_required(VERSION 3.28)`nproject(Consumer LANGUAGES CXX)`nfind_package(FrameGraphLab 0.1 CONFIG REQUIRED)`nadd_executable(consumer main.cpp)`ntarget_link_libraries(consumer PRIVATE FrameGraphLab::framegraph_core)`n")
    [IO.File]::WriteAllText((Join-Path $consumer 'main.cpp'),"#include <framegraph/plan.hpp>`nint main(){return framegraph::PlanCompiler::compile({},{}).has_value()?0:1;}`n")
    cmake -S $consumer -B (Join-Path $consumer 'build') -G Ninja "-DCMAKE_PREFIX_PATH=$prefix" -DCMAKE_CXX_COMPILER=clang++;if($LASTEXITCODE){throw 'consumer configure'}
    cmake --build (Join-Path $consumer 'build');if($LASTEXITCODE){throw 'consumer build'}
    & (Join-Path $consumer 'build/consumer.exe');if($LASTEXITCODE){throw 'consumer run'}
  }
  $env:ASAN_OPTIONS='halt_on_error=1';$env:UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1'
  Step sanitizers {cmake --preset core-sanitized --fresh;if($LASTEXITCODE){throw 'configure'};cmake --build --preset core-sanitized;if($LASTEXITCODE){throw 'build'};ctest --preset core-sanitized;if($LASTEXITCODE){throw 'ctest'};& './build/core-sanitized/framegraph_property.exe' --cases 10000;if($LASTEXITCODE){throw 'sanitized property'}}
  Step executor-probe {python tests/d3d12/validate_executor.py .mqb/bin/FrameGraphLab-release.exe warp;if($LASTEXITCODE){throw 'probe'}}
  Step culling-warp {python tests/d3d12/validate_culling.py .mqb/bin/FrameGraphLab-release.exe warp;if($LASTEXITCODE){throw 'culling warp'}}
  Step scene-warp {python tests/d3d12/validate_scene.py .mqb/bin/FrameGraphLab-release.exe warp;if($LASTEXITCODE){throw 'scene warp'}}
  Step negatives {python tests/d3d12/runtime_negative.py .mqb/bin/FrameGraphLab-release.exe;if($LASTEXITCODE){throw 'runtime negative'};python tests/d3d12/executor_negative.py .mqb/bin/FrameGraphLab-release.exe;if($LASTEXITCODE){throw 'executor negative'}}
  if(-not $SkipHardware){
    Step culling-hardware {python tests/d3d12/validate_culling.py .mqb/bin/FrameGraphLab-release.exe hardware;if($LASTEXITCODE){throw 'culling hardware'}}
    Step scene-hardware {python tests/d3d12/validate_scene.py .mqb/bin/FrameGraphLab-release.exe hardware;if($LASTEXITCODE){throw 'scene hardware'}}
    Step runtime-benchmark {python tools/benchmark_runtime.py .mqb/bin/FrameGraphLab-release.exe --backend hardware --samples 7 --frames 240 --expected-sha $head --output artifacts/benchmarks/runtime-hardware-final.json;if($LASTEXITCODE){throw 'runtime benchmark'}}
    Step hardware-interactive {& './.mqb/bin/FrameGraphLab-release.exe' --hardware --frames 240 --report artifacts/reports/hardware-interactive.json;if($LASTEXITCODE){throw 'interactive'}}
    Step hardware-stress {& './.mqb/bin/FrameGraphLab-release.exe' --hardware --headless --frames 1000 --report artifacts/reports/hardware-stress.json;if($LASTEXITCODE){throw 'hardware stress'}}
    Step resize {& './.mqb/bin/FrameGraphLab-release.exe' --hardware --headless --frames 100 --resize-stress --report artifacts/reports/resize-stress.json;if($LASTEXITCODE){throw 'resize'}}
  }
  Step warp-stress {& './.mqb/bin/FrameGraphLab-release.exe' --warp --headless --frames 1000 --report artifacts/reports/warp-stress.json;if($LASTEXITCODE){throw 'warp stress'}}
  Step primary-artifacts {& './.mqb/bin/FrameGraphLab-release.exe' --warp --headless --frames 240 --scene-seed 24301 --capture artifacts/captures/neon-ruins.png --rgba artifacts/captures/neon-ruins.rgba --report artifacts/reports/frame-report.json --plan artifacts/reports/framegraph-plan.json;if($LASTEXITCODE){throw 'primary'};python tools/build_inspector.py --plan artifacts/reports/framegraph-plan.json --report artifacts/reports/frame-report.json --image artifacts/captures/neon-ruins.png --output artifacts/viewer/framegraph-inspector.html;if($LASTEXITCODE){throw 'inspector'};python tests/viewer/validate_inspector.py --plan artifacts/reports/framegraph-plan.json --report artifacts/reports/frame-report.json --image artifacts/captures/neon-ruins.png --html artifacts/viewer/framegraph-inspector.html;if($LASTEXITCODE){throw 'viewer'}}
  Step package {./scripts/package.ps1 -Configuration release;if($LASTEXITCODE){throw 'package'}}
  $manifest=Get-Content artifacts/release/DELIVERY_MANIFEST.json -Raw|ConvertFrom-Json;$win=(Join-Path $root ($manifest.packages|Where-Object{$_.path -like '*Win64*'}).path)
  Step clean-extraction {./scripts/validate-package.ps1 -Zip $win -ExpectedSha $head;if($LASTEXITCODE){throw 'extraction'}}
  Step no-residue {if(Get-Process -Name 'FrameGraphLab*' -ErrorAction SilentlyContinue){throw 'FrameGraphLab process remains'}}
}finally{
  $summary=[ordered]@{schema_version=1;git_sha=$head;source_clean=(-not[bool](git status --porcelain));completed_utc=[DateTime]::UtcNow.ToString('o');steps=$steps}
  [IO.File]::WriteAllText((Join-Path $out 'summary.json'),($summary|ConvertTo-Json -Depth 8))
}
$summary|ConvertTo-Json -Depth 8
