#!/usr/bin/env bash
# Confirms the service starts and answers. It asserts nothing about security,
# which is the point: a green pipeline today means the code compiled.
set -uo pipefail
cd "$(dirname "$0")/.."
make >/dev/null
./build/vulnshop config/sandbox.ini >/tmp/vulnshop-smoke.log 2>&1 &
PID=$!
sleep 1
CODE=$(curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8080/shop || echo 000)
kill $PID 2>/dev/null || true
[ "$CODE" = "200" ] && { echo "smoke test OK"; exit 0; }
echo "smoke test FAILED (http $CODE)"; exit 1
