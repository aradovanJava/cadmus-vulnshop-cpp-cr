# CADMUS-CR-12 · CR1 · Task 0 — environment check (VM1, Windows 11)
#
#   powershell -ExecutionPolicy Bypass -File tools\verify-env.ps1
#
# Run this first. If anything reports MISSING, tell your instructor before
# starting rather than losing lab time to a broken toolchain.

$ErrorActionPreference = "Continue"
$ok = $true

function Check($name, $cmd, $arg) {
    Write-Host -NoNewline ("{0,-16}" -f $name)
    try {
        $out = & $cmd $arg 2>&1 | Select-Object -First 1
        Write-Host "OK    $out" -ForegroundColor Green
    } catch {
        Write-Host "MISSING" -ForegroundColor Red
        $script:ok = $false
    }
}

Write-Host "`nCADMUS-CR-12 CR1 - environment check`n" -ForegroundColor Cyan

Check "g++"      "g++"      "--version"
Check "make"     "make"     "--version"
Check "cmake"    "cmake"    "--version"
Check "git"      "git"      "--version"
Check "docker"   "docker"   "--version"
Check "snyk"     "snyk"     "--version"
Check "curl"     "curl"     "--version"

Write-Host -NoNewline ("{0,-16}" -f "sonarqube")
try {
    $r = Invoke-WebRequest -Uri "http://localhost:9000/api/system/status" -UseBasicParsing -TimeoutSec 5
    Write-Host "OK    $($r.Content)" -ForegroundColor Green
} catch {
    Write-Host "NOT RUNNING - start it with: docker compose up -d sonarqube" -ForegroundColor Yellow
}

Write-Host -NoNewline ("{0,-16}" -f "build-wrapper")
if (Get-Command build-wrapper-win-x86-64.exe -ErrorAction SilentlyContinue) {
    Write-Host "OK" -ForegroundColor Green
} else {
    Write-Host "MISSING - download it from SonarQube > Project > C/C++ > Manual" -ForegroundColor Red
    $ok = $false
}

Write-Host ""
if ($ok) { Write-Host "Toolchain ready." -ForegroundColor Green }
else     { Write-Host "Something is missing - raise it now." -ForegroundColor Red }
