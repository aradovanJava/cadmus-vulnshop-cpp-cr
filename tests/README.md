# tests/

`corpus/` holds the partner-feed frames used in CR2 to demonstrate that the
parser fix holds. Each file is a raw frame; replay them against a build with
sanitizers enabled:

```bash
make asan
for f in tests/corpus/*.bin; do
  curl -s -X POST --data-binary @"$f" -H 'X-User-Id: alice' \
       http://127.0.0.1:8080/api/partner/feed >/dev/null
done
```

Before the fix, ASan reports a write past the end of `PriceFrame` on
`oversize-length.bin`. After the fix, every frame is either parsed or rejected
and the sanitizer stays quiet. That difference is the acceptance criterion —
not the absence of a crash, which proves nothing.

| File | What it is |
|---|---|
| `well-formed.bin` | A valid frame. Must parse identically before and after. |
| `oversize-length.bin` | Length field larger than the frame. Triggers W-20. |
| `wrap-length.bin` | Length chosen so that `claimed + 1` wraps. Triggers W-21. |
| `unterminated-note.bin` | `note[32]` fully populated, no NUL. Triggers W-22. |
