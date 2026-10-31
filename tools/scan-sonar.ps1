# CADMUS-CR-12 · CR1 · Task 2 — static analysis.
#
#   powershell -ExecutionPolicy Bypass -File tools\scan-sonar.ps1 -Token sqp_xxx
#
# The build wrapper is not optional. SonarQube's C/C++ analyser inspects the
# compilation, not the source tree: without it the scan reports a fraction of
# the findings and looks exactly like a clean result.
param([Parameter(Mandatory=$true)][string]$Token)

Set-Location (Join-Path $PSScriptRoot "..")

Write-Host "[1/3] Clean" -ForegroundColor Cyan
make clean | Out-Null

Write-Host "[2/3] Build under the wrapper" -ForegroundColor Cyan
build-wrapper-win-x86-64.exe --out-dir bw-output make

Write-Host "[3/3] Analyse" -ForegroundColor Cyan
sonar-scanner "-Dsonar.token=$Token"

Write-Host ""
Write-Host "Open http://localhost:9000 and select cadmus-vulnshop-cpp-cr." -ForegroundColor Green
Write-Host "Issues and Security Hotspots are separate tabs. They are different kinds of claim."
