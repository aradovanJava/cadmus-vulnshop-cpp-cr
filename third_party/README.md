# third_party/

Three dependencies, vendored as source directly into the repository.

| Directory | Claimed version | Copied | Known issue in that version |
|---|---|---|---|
| `zlib-1.2.11/` | 1.2.11 | 2019, three local edits, never re-synced | CVE-2018-25032 — memory corruption in `deflate` |
| `libpng-1.6.35/` | 1.6.35 | 2019, unmodified | CVE-2019-7317 — use-after-free in `png_image_free` |
| `json-3.7.3/` | 3.7.3 | 2020, dropped in whole | — |

These directories contain small stand-ins rather than the real upstream
sources, so that the repository stays small and buildable offline. The
*shape* of the problem is the real one, and it is the point of excerpt C:

- **There is no manifest.** No `vcpkg.json`, no `conanfile.txt`, no lockfile.
  A composition-analysis tool that reads a manifest has nothing to read here.
- **The version string lies.** `zlib-1.2.11/` carries local edits, so it is
  neither 1.2.11 nor anything else with a published advisory history.
- **The build links statically.** A distribution patch to zlib reaches a
  dynamically linked service on the next restart. It never reaches this binary.
- **Nobody can say which of the three is still called** from application code.
  An unused vulnerable dependency and a heavily used one produce identical
  scanner output and completely different risk.

CR3 asks you to answer all four of those points, produce an SBOM, and move
the dependency set onto a manifest-based package manager.
