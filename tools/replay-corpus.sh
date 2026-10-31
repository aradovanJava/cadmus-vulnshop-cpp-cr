#!/usr/bin/env bash
# Exercises the request surface. Every request below is an ordinary HTTP
# request; none of them is an exploit payload. They exist so that a dynamic
# analyser has something to observe.
set -uo pipefail
H="${1:-http://127.0.0.1:8080}"
C="curl -s -o /dev/null -m 5"

echo "browsing the storefront"
$C "$H/shop"
$C "$H/shop/product?id=1"
$C "$H/shop/product?id=3"

echo "searching"
$C "$H/shop/search?q="
$C "$H/shop/search?q=cable"
$C "$H/shop/search?q=$(printf 'A%.0s' {1..400})"

echo "account area"
$C -X POST -d 'user=alice&pass=hunter2' "$H/account/login"
$C "$H/account/orders" -H 'Cookie: vs_remember=alice:user'
$C "$H/account/order?id=1001" -H 'Cookie: vs_remember=alice:user'
$C "$H/account/profile" -H 'Cookie: vs_remember=alice:user'

echo "support desk"
$C -X POST -d 'subject=printer+jam&body=help' "$H/support/ticket"
$C "$H/support/attachment?file=invoice.txt"

echo "admin pages"
$C "$H/admin/users"
$C "$H/admin/product/edit?id=1"

echo "api"
$C -H 'X-User-Id: alice' "$H/api/orders/1000"
$C -H 'X-User-Id: alice' "$H/api/reports/orders.csv"
printf 'AAAA' | $C --data-binary @- -H 'X-User-Id: alice' "$H/api/import/prices"
printf 'AB'   | $C --data-binary @- -H 'X-User-Id: alice' "$H/api/import/prices"

echo "corpus replayed"
