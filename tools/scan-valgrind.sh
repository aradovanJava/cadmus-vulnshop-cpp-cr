#!/usr/bin/env bash
# CADMUS-CR-12 · CR1 · Task 4 — dynamic analysis under Memcheck.
#
#   docker compose run --rm valgrind
#
# Starts the application under Valgrind, replays the request corpus against it,
# then stops it cleanly so that the leak summary is produced.
set -uo pipefail
cd "$(dirname "$0")/.."

mkdir -p lab /var/lib/vulnshop/attachments
echo "invoice 4711" > /var/lib/vulnshop/attachments/invoice.txt

echo "[1/3] Build"
make

echo "[2/3] Start under Memcheck"
valgrind --leak-check=full --show-leak-kinds=definite --track-origins=yes \
         --log-file=lab/valgrind-report.txt \
         ./build/vulnshop config/sandbox.ini &
VG=$!
sleep 8

echo "[3/3] Replay the request corpus"
./tools/replay-corpus.sh http://127.0.0.1:8080

kill -TERM $VG 2>/dev/null
wait $VG 2>/dev/null

echo
echo "Report written to lab/valgrind-report.txt"
grep -E "Invalid (read|write)|definitely lost|Conditional jump" lab/valgrind-report.txt | head -20 || true
