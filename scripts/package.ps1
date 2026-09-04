param([ValidateSet('debug','release')][string]$Configuration='release')
$ErrorActionPreference='Stop'
$projectRoot=[IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
Set-Location -LiteralPath $projectRoot
if(git status --porcelain){throw 'Packaging requires a clean working tree'}
$head=(git rev-parse HEAD).Trim(); if($head -notmatch '^[0-9a-f]{40}$'){throw 'Invalid Git HEAD'}
$short=$head.Substring(0,12)
$releaseRoot=[IO.Path]::GetFullPath((Join-Path $projectRoot 'artifacts/release'))
if(-not $releaseRoot.StartsWith($projectRoot+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Release path escaped project'}
New-Item -ItemType Directory -Path $releaseRoot -Force|Out-Null
function Reset-OwnedDirectory([string]$Path){
  $resolved=[IO.Path]::GetFullPath($Path)
  if($resolved -eq $releaseRoot -or -not $resolved.StartsWith($releaseRoot+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw "Unsafe package directory: $resolved"}
  if(Test-Path -LiteralPath $resolved){Remove-Item -LiteralPath $resolved -Recurse -Force}
  New-Item -ItemType Directory -Path $resolved|Out-Null
  return $resolved
}
$stage=Reset-OwnedDirectory (Join-Path $releaseRoot 'staging-win64')
./scripts/build.ps1 app -Configuration $Configuration
if($LASTEXITCODE -ne 0){throw 'MQB app build failed'}
$binary=Join-Path $projectRoot ".mqb/bin/FrameGraphLab-$Configuration.exe"
if(-not(Test-Path -LiteralPath $binary)){throw 'MQB application output missing'}
Copy-Item -LiteralPath $binary -Destination (Join-Path $stage 'FrameGraphLab.exe')
Copy-Item -LiteralPath LICENSE -Destination $stage
New-Item -ItemType Directory -Path (Join-Path $stage 'shaders')|Out-Null
foreach($shader in (Get-Content build-manifest.json -Raw|ConvertFrom-Json).assets.shaders){
  $source=Join-Path $projectRoot $shader
  Copy-Item -LiteralPath $source -Destination (Join-Path $stage 'shaders')
}
foreach($pair in @(
  @('artifacts/reports/framegraph-plan.json','sample-plan.json'),
  @('artifacts/reports/frame-report.json','sample-report.json'),
  @('artifacts/viewer/framegraph-inspector.html','framegraph-inspector.html'),
  @('artifacts/captures/neon-ruins.png','neon-ruins.png'))){
  if(-not(Test-Path -LiteralPath $pair[0])){throw "Missing primary artifact: $($pair[0])"}
  Copy-Item -LiteralPath $pair[0] -Destination (Join-Path $stage $pair[1])
}
$quick=@'
# FrameGraphLab 0.1.0 quick start

Interactive hardware: `FrameGraphLab.exe --hardware`

Bounded WARP evidence: `FrameGraphLab.exe --warp --headless --frames 240 --scene-seed 24301 --capture smoke.png --report smoke.json --plan plan.json`

Controls: arrows/left drag camera; Space pause; N step; R reset; A alias; G GPU/CPU draw; V Final/HDR/Bloom.
The executable compiles HLSL from the adjacent shaders directory and requires Windows 10+, D3D12/WARP, and the Debug Layer system component for validation policy.
'@
[IO.File]::WriteAllText((Join-Path $stage 'QUICK_START.md'),$quick)
$config=[ordered]@{adapter='auto';width=1280;height=720;frames_in_flight=3;scene_seed=24301;aliasing=$true;debug_layer_required=$true}
[IO.File]::WriteAllText((Join-Path $stage 'sample-config.json'),($config|ConvertTo-Json))
function Entry([IO.FileInfo]$File,[string]$Base){[ordered]@{path=[IO.Path]::GetRelativePath($Base,$File.FullName).Replace('\','/');bytes=$File.Length;sha256=(Get-FileHash -LiteralPath $File.FullName -Algorithm SHA256).Hash.ToLowerInvariant()}}
$files=@(Get-ChildItem -LiteralPath $stage -Recurse -File|Sort-Object FullName|ForEach-Object{Entry $_ $stage})
$inner=[ordered]@{schema_version=1;product='FrameGraphLab';version='0.1.0';git_sha=$head;source_clean=$true;configuration=$Configuration;files=$files;note='Package SHA is recorded by the outer release manifest to avoid self-reference.'}
[IO.File]::WriteAllText((Join-Path $stage 'DELIVERY_MANIFEST.json'),($inner|ConvertTo-Json -Depth 8))
$winZip=Join-Path $releaseRoot "FrameGraphLab-Win64-0.1.0-$short.zip"
$sourceZip=Join-Path $releaseRoot "FrameGraphLab-Source-0.1.0-$short.zip"
foreach($file in @($winZip,$sourceZip)){if(Test-Path -LiteralPath $file){Remove-Item -LiteralPath $file -Force}}
Compress-Archive -Path (Join-Path $stage '*') -DestinationPath $winZip -CompressionLevel Optimal
git archive --format=zip --output=$sourceZip HEAD
if($LASTEXITCODE -ne 0){throw 'git archive failed'}
$packages=@(Get-Item $winZip,$sourceZip|ForEach-Object{Entry $_ $projectRoot})
$innerHash=(Get-FileHash -LiteralPath (Join-Path $stage 'DELIVERY_MANIFEST.json') -Algorithm SHA256).Hash.ToLowerInvariant()
$outer=[ordered]@{schema_version=1;product='FrameGraphLab';version='0.1.0';git_sha=$head;source_clean=$true;source_only_github_release=$false;binary_upload_authorized=$true;github_release_assets=@([IO.Path]::GetRelativePath($projectRoot,$winZip).Replace('\','/'),[IO.Path]::GetRelativePath($projectRoot,$winZip).Replace('\','/')+'.sha256','artifacts/release/DELIVERY_MANIFEST.json');inner_manifest_sha256=$innerHash;packages=$packages}
$outerPath=Join-Path $releaseRoot 'DELIVERY_MANIFEST.json'
[IO.File]::WriteAllText($outerPath,($outer|ConvertTo-Json -Depth 8))
foreach($package in $packages){$name=[IO.Path]::GetFileName($package.path);[IO.File]::WriteAllText((Join-Path $releaseRoot ($name+'.sha256')),($package.sha256+' *'+$name+"`n"))}
$outer|ConvertTo-Json -Depth 8
