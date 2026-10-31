# Architecture

This is the system described in **excerpt E** of node N1 — the one with no
vulnerable line of code anywhere in it.

```
  Web client  ──►  API gateway  ──►  Order service  ──►  Pricing daemon
   SPA, public      authenticates     trusts gateway      internal only
                                            │
                                            ▼
                                       Partner feed
                                      binary protocol
```

## What each component does

**API gateway.** Terminates TLS, validates the caller's token, and forwards the
request downstream — including the raw token *and* an `X-User-Id` header it
sets itself.

**Order service** (this repository). Reads `X-User-Id` and treats it as the
caller's identity. It never re-validates the token. Nothing prevents a request
from reaching it without passing through the gateway.

**Pricing daemon.** No authentication. It listens on a UNIX socket with mode
`0666` because it is "internal", on a flat network.

**Partner feed.** A third party's binary protocol. Frames are read into a
packed struct with `memcpy`, using a length field taken from the frame, and
`reinterpret_cast` to domain types.

**Process model.** TLS termination, frame parsing and business logic all run in
one process, as root. No privilege separation, no seccomp filter, no sandbox.

**Observability.** All five components log to one stream. Nothing raises an
alert.

## The six weaknesses

| # | Weakness | Category | Question that finds it |
|---|---|---|---|
| 1 | `X-User-Id` trusted between services | A01 · A06 | Q3 |
| 2 | Pricing daemon unauthenticated, socket `0666` | A01 · A02 | Q3 |
| 3 | Partner frames `memcpy`'d into a packed struct | A08 | Q5 |
| 4 | One process, as root, no sandbox | A02 · A06 | Q5 |
| 5 | Raw token forwarded downstream | A07 · A01 | Q3 |
| 6 | One log stream, no alerting | A09 | Q4 |

Rows 3 and 4 are the ones with no equivalent in a memory-safe language. In a
managed runtime, a parser defect is an exception. Here, the blast radius of a
single memory bug in the parser is the whole service and the host — which is
why privilege separation is an *architectural* control, not a hardening detail.

## The reference designs to point at

- **OpenSSH privilege separation** — untrusted protocol parsing in an
  unprivileged, chrooted child that holds no keys.
- **Browser renderer sandboxes** — every participant has used one today
  without thinking about why it exists.
