param(
    [Parameter(Position = 0)][string]$Target = 'plan',
    [ValidateSet('debug', 'release')][string]$Configuration = 'debug',
    [switch]$Run,
    [Parameter(ValueFromRemainingArguments = $true)][string[]]$ProgramArguments
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $projectRoot
$scratch = Join-Path $projectRoot 'artifacts/tmp'
New-Item -ItemType Directory -Path $scratch -Force | Out-Null
$env:TEMP = $scratch
$env:TMP = $scratch
python tools/build_contract.py
if ($LASTEXITCODE -ne 0) { throw 'Build contract check failed' }
$manifest = Get-Content -LiteralPath 'build-manifest.json' -Raw | ConvertFrom-Json -AsHashtable
if (-not $manifest.targets.ContainsKey($Target)) { throw "Unknown target: $Target" }
$definition = $manifest.targets[$Target]
if ($Target -eq 'app') {
    python tools/write_revision.py
    if ($LASTEXITCODE -ne 0) { throw 'Revision generation failed' }
}
$sources = @($definition.sources)
foreach ($group in $definition.groups) { $sources += @($manifest.groups[$group]) }
if (-not $sources.Count) { throw 'Target has no source files' }
if ($Run -and $definition.type -ne 'exe') { throw 'Only executable targets can run' }
$profile = if ($definition.type -eq 'exe') { "$Target-$Configuration" } else { $Configuration }
$outputName = if ($Target -eq 'app') { "FrameGraphLab-$Configuration" } else { "framegraph_$Target-$Configuration" }
$mqbArguments = @($(if ($Run) { 'run' } else { 'build' })) + $sources + @('--profile', $profile, '--type', $definition.type, '--output', $outputName, '--jobs', '4')
if ($sources.Count -eq 1) { $mqbArguments += '--no-discover' }
if ($ProgramArguments.Count) {
    if (-not $Run) { throw 'Program arguments require -Run' }
    $mqbArguments += '--'
    $mqbArguments += $ProgramArguments
}
& mqb @mqbArguments
exit $LASTEXITCODE
