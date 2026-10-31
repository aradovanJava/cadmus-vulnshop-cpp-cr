# cadmus-vulnshop-cpp-cr

The CR1 training target for **CADMUS-CR-12 — Secure Coding C/C++**.

> **Read [SAFETY.md](SAFETY.md) before you start it.** This program is
> intentionally exploitable and must stay inside the Cyber Range sandbox.

This is the CR fork of the application you read excerpts from in N1 and N2. It
adds a server-rendered tier — a storefront, an account area, a support desk and
administrative pages — alongside the JSON API, and a set of memory-safety
defects that only the dynamic analysis tier reports.

## Start here

| You are | Read |
|---|---|
| A participant | [`lab/LAB_GUIDE.md`](lab/LAB_GUIDE.md), then fill in [`lab/FINDINGS.md`](lab/FINDINGS.md) |
| An instructor | `docs/ANSWER_KEY.md` (instructor build only) |

## Build and run

```bash
make                                   # or: cmake -S . -B build && cmake --build build
./tools/run-app.ps1                    # Windows, VM1
./build/vulnshop config/sandbox.ini    # anywhere else - loopback only
```

Then open <http://127.0.0.1:8080/shop>. Training accounts `alice`, `bob`,
`carol` and `support` share the password shown on the sign-in page.

## The analysis toolchain

| Tool | Command | Finds |
|---|---|---|
| SonarQube | `tools/scan-sonar.ps1 -Token …` | Injection sinks, weak cryptography, unescaped output |
| Snyk | `tools/scan-snyk.ps1` | Advisories for components it can identify — read the denominator |
| Valgrind | `docker compose run --rm valgrind` | Out-of-bounds access, use after free, double free, leaks |
| Sanitizers | `make asan` then replay the corpus | The same, faster, with better stack traces |

`tools/replay-corpus.sh` exercises the request surface so the dynamic tools have
something to observe. Every request in it is an ordinary HTTP request.

## Layout

```
include/vulnshop.h        config, domain types, request/response
src/main.cpp              startup, configuration, route registration
src/web/                  JSON API and server-rendered controllers
src/util/templ.cpp        page rendering - several weaknesses live here
src/auth/                 session store, authorisation service
src/db/                   in-memory store with a naive SQL front door
src/parse/                partner feed binary parser
third_party/              three vendored dependencies, no manifest
config/                   vulnshop.ini (the exercise), sandbox.ini (safe)
tools/                    environment check, run, scan, corpus replay
lab/                      LAB_GUIDE.md and the FINDINGS.md you submit
```

## What this repository is for

CR1 asks you to **find and classify**, not to fix. CR2 fixes it, CR3 remediates
the dependency tree, CR4 builds the pipeline that stops any of it returning.

`VULNERABILITY_MAP.md` is the complete inventory. It is included in the
participant build on purpose — the exercise is not a treasure hunt, and being
able to check your findings against the map afterwards is part of the learning.
Do not read it before the debrief.

---

Funded by the European Union under the Digital Europe Programme ·
Grant Agreement No. 101190006 · Supported by HaDEA.
