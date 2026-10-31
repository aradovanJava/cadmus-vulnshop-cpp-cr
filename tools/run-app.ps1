# CADMUS-CR-12 · CR1 — build and run the target application.
#
#   powershell -ExecutionPolicy Bypass -File tools\run-app.ps1
#
# Binds to 127.0.0.1 using config\sandbox.ini. Do not change that.
Set-Location (Join-Path $PSScriptRoot "..")
Write-Host "Building..." -ForegroundColor Cyan
make
if ($LASTEXITCODE -ne 0) { Write-Host "Build failed." -ForegroundColor Red; exit 1 }
Write-Host "Starting on http://127.0.0.1:8080/shop  (Ctrl-C to stop)" -ForegroundColor Green
.\build\vulnshop config\sandbox.ini
