param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",

    [ValidateSet("MidiScorer", "MidiScorerTests", "All")]
    [string]$Target = "All",

    [string]$JuceRoot = "",

    [switch]$RunTests
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
$buildDir = Join-Path $repoRoot "build"

if ([string]::IsNullOrWhiteSpace($JuceRoot)) {
    $JuceRoot = $env:JUCE_ROOT
}
if ([string]::IsNullOrWhiteSpace($JuceRoot) -and (Test-Path "C:/JUCE/CMakeLists.txt")) {
    $JuceRoot = "C:/JUCE"
}
if ([string]::IsNullOrWhiteSpace($JuceRoot)) {
    $JuceRoot = Join-Path $repoRoot ".deps/JUCE"
}

if (-not (Test-Path (Join-Path $JuceRoot "CMakeLists.txt"))) {
    throw "JUCE not found at '$JuceRoot'. Pass -JuceRoot, clone .deps/JUCE, or set JUCE_ROOT."
}

Write-Host "==> MidiScorer Windows build"
Write-Host "    Repo:          $repoRoot"
Write-Host "    Build dir:     $buildDir"
Write-Host "    Configuration: $Configuration"
Write-Host "    JUCE_ROOT:     $JuceRoot"

cmake -S $repoRoot -B $buildDir -DJUCE_ROOT="$JuceRoot"

$targets = @()
switch ($Target) {
    "MidiScorer" { $targets = @("MidiScorer") }
    "MidiScorerTests" { $targets = @("MidiScorerTests") }
    default { $targets = @("MidiScorer", "MidiScorerTests") }
}

cmake --build $buildDir --config $Configuration --target $targets

$shouldRunTests = $RunTests.IsPresent -or $Target -eq "All" -or $Target -eq "MidiScorerTests"
if ($shouldRunTests) {
    ctest --test-dir $buildDir -C $Configuration --output-on-failure
}

if ($targets -contains "MidiScorer") {
    Write-Host ""
    Write-Host "Built app:"
    Write-Host "  $buildDir/MidiScorer_artefacts/$Configuration/MidiScorer.exe"
}
