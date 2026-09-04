param([Parameter(Mandatory)][string]$Zip,[Parameter(Mandatory)][string]$ExpectedSha)
$ErrorActionPreference='Stop'
$projectRoot=[IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot));Set-Location -LiteralPath $projectRoot
$releaseRoot=[IO.Path]::GetFullPath((Join-Path $projectRoot 'artifacts/release'))
$zipPath=[IO.Path]::GetFullPath($Zip)
if(-not $zipPath.StartsWith($releaseRoot+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Package must be under owned release root'}
$extract=[IO.Path]::GetFullPath((Join-Path $releaseRoot 'clean-extraction'))
if(-not $extract.StartsWith($releaseRoot+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Extraction path escaped release root'}
if(Test-Path -LiteralPath $extract){Remove-Item -LiteralPath $extract -Recurse -Force}
New-Item -ItemType Directory -Path $extract|Out-Null
Expand-Archive -LiteralPath $zipPath -DestinationPath $extract
Push-Location $extract
try{
  & './FrameGraphLab.exe' --warp --headless --frames 240 --capture smoke.png --report smoke.json --plan plan.json
  if($LASTEXITCODE -ne 0){throw 'Extracted WARP smoke failed'}
  $report=Get-Content smoke.json -Raw|ConvertFrom-Json;$plan=Get-Content plan.json -Raw|ConvertFrom-Json
  if(-not $report.success -or $report.frames -ne 240 -or $report.git_sha -ne $ExpectedSha -or -not $report.source_clean){throw 'Extracted report provenance/result failed'}
  if($report.alias_reuse_events -lt 1 -or $report.actual_heap_bytes -ne $report.planned_heap_bytes){throw 'Extracted alias/heap evidence failed'}
  if($report.debug_errors+$report.debug_warnings+$report.debug_corruptions -ne 0){throw 'Extracted Debug Layer gate failed'}
  if($report.plan_identity -ne $plan.plan_identity -or -not $plan.plan_identity){throw 'Extracted plan identity failed'}
  $png=[IO.File]::ReadAllBytes((Join-Path $extract 'smoke.png'))
  if($png.Length -lt 24 -or [BitConverter]::ToString($png[0..7]) -ne '89-50-4E-47-0D-0A-1A-0A'){throw 'Extracted PNG invalid'}
  foreach($path in @('smoke.png','smoke.json','plan.json')){if(-not[IO.Path]::GetFullPath((Join-Path $extract $path)).StartsWith($extract+[IO.Path]::DirectorySeparatorChar)){throw 'Output path escaped extraction'}}
  [ordered]@{success=$true;git_sha=$report.git_sha;pixel_hash=$report.pixel_hash;plan_identity=$report.plan_identity;actual_heap_bytes=$report.actual_heap_bytes;saved_bytes=$report.saved_bytes;debug=@($report.debug_errors,$report.debug_warnings,$report.debug_corruptions);extraction=[IO.Path]::GetRelativePath($projectRoot,$extract)}|ConvertTo-Json
}finally{Pop-Location}
