# SAFETY — read before you start the application

`cadmus-vulnshop-cpp-cr` is **deliberately vulnerable**. It contains working
authorisation bypasses, injection points, memory-corruption defects and a
configuration written to be as unsafe as a plausible real one.

## Rules

1. **Sandbox only.** Run it inside the Cyber Range VM, or on a machine you are
   prepared to treat as compromised.
2. **Never expose it to a network you care about.** The built-in default binds
   to `127.0.0.1`. `config/vulnshop.ini` overrides that with `0.0.0.0` — that
   override is itself one of the findings. Outside the range, use
   `config/sandbox.ini`.
3. **Do not run it as root** outside the range. The Dockerfile does, and that is
   a finding, not a recommendation.
4. **Do not point it at real data or real credentials.** The store is in memory
   and seeded with fabricated records.
5. **Do not copy code from this repository into anything real.** Several patterns
   here look like reasonable defensive engineering and are not.
6. **Core dumps.** `config/vulnshop.ini` enables them. Clear `core*` files when
   you are finished.

## What "vulnerable" means here

The defects are the kind that appear in code review and in scanner output, at a
size you can read in a minute. There are no exploit payloads, no shellcode and
no offensive tooling in this repository. The requests in `tools/` and in the lab
guide are ordinary HTTP requests whose only purpose is to show that a finding is
reachable rather than theoretical.

## Reporting

If you find a weakness that is not in `VULNERABILITY_MAP.md`, tell your
instructor. Undocumented findings are welcome — the map is a teaching inventory,
not a claim of completeness.
