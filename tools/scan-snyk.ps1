# CADMUS-CR-12 · CR1 · Task 3 — composition analysis.
#
#   powershell -ExecutionPolicy Bypass -File tools\scan-snyk.ps1
#
# Produces three artefacts in lab\. Read the DENOMINATOR of the dependency
# report before you read its findings.
Set-Location (Join-Path $PSScriptRoot "..")
New-Item -ItemType Directory -Force -Path lab | Out-Null

Write-Host "[1/3] Declared dependencies" -ForegroundColor Cyan
Get-Content vcpkg.json | Out-File -Encoding utf8 lab\declared-deps.txt

Write-Host "[2/3] What is actually compiled and linked" -ForegroundColor Cyan
Get-ChildItem -Recurse third_party -Include *.c,*.h |
    Select-Object -ExpandProperty FullName |
    Out-File -Encoding utf8 lab\vendored-sources.txt
Get-ChildItem third_party -Directory | ForEach-Object {
    "$($_.Name)  VERSION=$(Get-Content (Join-Path $_.FullName 'VERSION') -ErrorAction SilentlyContinue)"
} | Out-File -Encoding utf8 lab\vendored-tree.txt

Write-Host "[3/3] Snyk" -ForegroundColor Cyan
snyk test --json-file-output=lab\snyk-deps.json
snyk code test --json-file-output=lab\snyk-code.json

Write-Host ""
Write-Host "Compare lab\declared-deps.txt with lab\vendored-tree.txt." -ForegroundColor Yellow
Write-Host "The difference between those two files is the point of this task."
