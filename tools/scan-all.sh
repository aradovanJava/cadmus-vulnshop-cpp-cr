#!/usr/bin/env bash
# CADMUS-CR-12 · CR1 · Linux/macOS equivalent of the PowerShell scripts.
set -euo pipefail
cd "$(dirname "$0")/.."
: "${SONAR_TOKEN:?Set SONAR_TOKEN first (SonarQube > My Account > Security)}"

mkdir -p lab

echo "[1/5] Build"
make clean && make

echo "[2/5] Sanitizer build and corpus replay"
make asan || true

echo "[3/5] SonarQube (build wrapper is not optional)"
build-wrapper-linux-x86-64 --out-dir bw-output make clean all
sonar-scanner -Dsonar.token="$SONAR_TOKEN"

echo "[4/5] Snyk"
snyk test --json-file-output=lab/snyk-deps.json || true
snyk code test --json-file-output=lab/snyk-code.json || true

echo "[5/5] Valgrind"
./tools/scan-valgrind.sh || true

echo
echo "Open http://localhost:9000 and select cadmus-vulnshop-cpp-cr."
echo "Then read lab/valgrind-report.txt. They report different kinds of thing."
