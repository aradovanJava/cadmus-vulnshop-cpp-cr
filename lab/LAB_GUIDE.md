# CR1 · Vulnerability Identification & Code Analysis

**60 minutes · individual work · CADMUS-CR-12 Secure Coding C/C++**

You are a member of the Blue Team. A vulnerable C++ service has been handed to
you before it goes to production. Your task is not to fix it — that is CR2.
Your task is to **find what is wrong, classify it, and prove you looked**.

---

## What you are looking at

`cadmus-vulnshop-cpp-cr` is a C++17 order service with two tiers:

| Tier | Directory | What it is |
|---|---|---|
| JSON API | `src/web/order_controller.cpp`, `admin_controller.cpp`, `import_controller.cpp` | Endpoints under `/api/**`, used by a mobile client |
| Server-rendered | `src/web/shop_controller.cpp`, `account_controller.cpp`, `support_controller.cpp`, `admin_ui_controller.cpp` | The storefront, account area, support desk and admin pages |

Weaknesses are injected in **both tiers**, and several of them live in
`src/util/templ.cpp` — the rendering layer — rather than in any handler. A
reviewer who reads only the routing will miss them.

There is also a third category that has no equivalent in a managed language:
**memory-safety defects**, which no amount of reading the routes will reveal and
which the static analyser mostly does not report either. You have a tool for
those. Use it.

---

## Rules

- **Sandbox only.** The application binds to `127.0.0.1` when started with
  `config/sandbox.ini`. Do not change that, do not expose it, do not point it at
  real data. Read `SAFETY.md`.
- Work **individually**. You may ask the instructor questions; you may not
  compare findings until the debrief.
- `docs/ANSWER_KEY.md` is not in your copy of the repository. If you find it,
  you have the instructor build — tell them.

---

## Timeline

| Time | Task | Output |
|---|---|---|
| 00:00–00:05 | 0 · Environment check and orientation | toolchain confirmed |
| 00:05–00:15 | 1 · Build, run, and start the SonarQube scan | scan running |
| 00:15–00:25 | 2 · Triage the static analysis findings | classified findings |
| 00:25–00:35 | 3 · Composition analysis — what are you shipping? | 2 advisories assessed |
| 00:35–00:45 | 4 · Dynamic analysis — Valgrind and the sanitizers | ≥ 4 memory findings |
| 00:45–00:57 | 5 · Manual inspection — find what no tool reported | ≥ 4 further findings |
| 00:57–01:00 | 6 · Submit | `lab/FINDINGS.md` |

The SonarQube scan in Task 1 takes three to five minutes because the C/C++
analyser has to observe a full compilation. **Start it and then read code while
it runs.** Do not watch the progress bar.

---

## Task 0 — Environment check (5 min)

```powershell
powershell -ExecutionPolicy Bypass -File tools\verify-env.ps1
```

Everything should report `OK`. If SonarQube is not running:

```powershell
docker compose up -d sonarqube
```

Give it 60–90 seconds, then open <http://localhost:9000>. Log in with
`admin` / `admin` and set a throwaway password — this instance is disposable.

Create an analysis token: **My Account → Security → Generate Token**, type
*Global Analysis Token*. Copy it; you need it in Task 1.

---

## Task 1 — Build, run, and scan (10 min)

Start the application in one terminal:

```powershell
powershell -ExecutionPolicy Bypass -File tools\run-app.ps1
```

Open <http://127.0.0.1:8080/shop> and sign in as `alice`. All four training
accounts (`alice`, `bob`, `carol`, `support`) share the password shown on the
login page.

Spend three minutes **using the application as a user would**. Browse the shop,
open a product, add it to the cart, look at your orders, file a support ticket.
You are building a mental map of the request surface — you cannot review what
you cannot picture.

In a second terminal, start the scan:

```powershell
powershell -ExecutionPolicy Bypass -File tools\scan-sonar.ps1 -Token sqp_your_token
```

> **The build wrapper is not optional.** SonarQube's C/C++ analyser inspects the
> compilation, not the source tree. A scan run without it reports a fraction of
> the findings and looks exactly like a clean result. `scan-sonar.ps1` does this
> for you; if you run the scanner by hand, do not skip it.

While it runs, answer these three questions in `lab/FINDINGS.md`:

1. How many distinct HTTP entry points did you find by clicking around?
2. Which pages change behaviour depending on who is signed in?
3. Which page displays something that another user could have written?

Question 3 is doing real work. Hold on to your answer.

---

## Task 2 — Triage the static analysis findings (10 min)

Open <http://localhost:9000> and select **cadmus-vulnshop-cpp-cr**.

Two tabs matter, and they are different kinds of claim:

- **Issues** — the analyser asserts a defect.
- **Security Hotspots** — the analyser found a security-sensitive API and is
  *asking you* whether the context makes it dangerous. Accepting the default
  severity means not answering the question.

For each finding, record a verdict and **your own severity, decided before you
read the tool's**:

| Code | Meaning |
|---|---|
| `TP` | Real and reachable |
| `FP-scope` | Real pattern, wrong place — test code, generated code, dead code |
| `FP-context` | Safe here because of something the tool cannot see. Say what. |
| `DIS` | Disputed — you need a fact you do not have. Name the fact. |
| `NOT-SEC` | Real issue, not a security issue |

Two things to watch for specifically:

- At least one finding is reported at a **much lower severity than it deserves**,
  because the rule cannot know what the function it sits in is *for*. Read the
  surrounding code, not the snippet.
- At least one finding sits in a location where it **does not matter**. Deciding
  the correct disposition is more interesting than the verdict.

---

## Task 3 — Composition analysis (10 min)

```powershell
powershell -ExecutionPolicy Bypass -File tools\scan-snyk.ps1
```

**Record two numbers before you look at any advisory.** Count the dependencies
declared in `vcpkg.json`, and count the directories under `third_party/`. Then
count how many components Snyk says it tested.

The three numbers do not agree. The disagreement is the finding, and it is the
central lesson of this task: in C and C++ most dependencies arrive by a route
that produces no manifest, so a composition-analysis report describes the tool's
visibility rather than your exposure.

For each advisory Snyk does report, do **not** simply copy the severity. Answer
these four questions, stopping at the first *no*:

1. Is the vulnerable component present in what we actually build?
2. Is the vulnerable code path present in that copy?
3. Does application code call it, directly or transitively?
4. Can an attacker control the input that reaches it?

Question 2 has a twist here. Open `third_party/zlib-1.2.11/` and read the
source before you answer. The version number is a claim; check it.

> Record, for each advisory: the severity, your reachability verdict, and **the
> order in which you would actually fix them**. If your order differs from the
> severity order, say why. That sentence is the finding.

---

## Task 4 — Dynamic analysis (10 min)

This tier has no equivalent in the Java course, and it finds things nothing else
does.

```bash
docker compose run --rm valgrind
```

That builds the application, starts it under Memcheck, replays the request
corpus in `tools/replay-corpus.sh` against it, stops it cleanly so the leak
summary is produced, and writes `lab/valgrind-report.txt`.

If you prefer the sanitizers, and they are faster:

```bash
make asan
ASAN_OPTIONS=halt_on_error=0:detect_leaks=1 ./build/vulnshop-asan config/sandbox.ini &
./tools/replay-corpus.sh
```

Read the report for four distinct kinds of diagnostic:

| Diagnostic | What it means |
|---|---|
| `Invalid read` / `heap-buffer-overflow` | An access outside an allocation |
| `Invalid free` / `double-free` | A release that should not have happened |
| `Conditional jump depends on uninitialised value` | A decision taken on a value never written |
| `definitely lost` | A leak — only reported when the process exits cleanly |

For each one, record **the file and line**, and then answer the question the
tool cannot: *what request reached it?* The corpus tells you; the stack trace
tells you where.

> A clean run proves nothing. The corpus exercises the surface it exercises. If
> you have time, add a request to `tools/replay-corpus.sh` and run it again.

---

## Task 5 — Manual inspection (12 min)

Close the scanners. This is the part that decides the exercise.

Every weakness below is real and reachable, and **no tool in this lab reports
any of them**. Use the review checklist from N3 and the four absence questions:

- *How many attempts are allowed here?*
- *What is the base rule for an endpoint nobody listed?*
- *What alerts when this happens?*
- *What happens when this dependency is slow?*

Where to look, in order of yield:

| File | Ask |
|---|---|
| `src/main.cpp`, the pre-routing hook | Which paths does it gate? What does that mean for everything else? |
| `src/web/admin_ui_controller.cpp` | Nothing here checks a role. What hides the Admin link, and is that a control? |
| `src/util/templ.cpp`, `nav_for()` | Answers the previous question. |
| `src/auth/session.cpp` | Three ways to establish identity. How many of them can the browser edit? |
| `src/web/account_controller.cpp` | Two order handlers. One is scoped to the caller. Compare them. Then read the login handler for what is *absent*. |
| `src/web/shop_controller.cpp`, `/cart/add` | Which values does the server decide, and which does it accept from the form? |
| `src/util/templ.cpp`, `render_page()` | How is the external script loaded, and what happens if that CDN serves something else? |
| `CMakeLists.txt` | Read every flag. Which of them removes a mitigation? |

**Try things in the browser.** Sign in as `alice`, note your order number, then
change the number in the URL. Sign out entirely and navigate directly to
`/admin/users`. Edit the `vs_remember` cookie in the developer tools and reload.
The application is running — use it.

Target: **at least four** weaknesses that produced no tool finding. There are
more than thirty.

---

## Task 6 — Submit (3 min)

Complete `lab/FINDINGS.md` and save it. It must contain:

1. The three component counts from Task 3, and your remediation order with a reason.
2. Every tool finding you triaged, with your verdict and your severity.
3. Every memory finding, with file, line and the request that reached it.
4. Every weakness you found manually, with an OWASP Top 10:2025 category — or a
   CWE where no category fits.
5. **One finding written up in full** using the eight-field record from N3 —
   your choice, but choose the one you think is most serious.
6. Your final tally:
   - weaknesses the tools found: ____
   - weaknesses you found that the tools did not: ____

Point 6 is the number the debrief opens with.

---

## What CR1 is assessing

Not how many weaknesses you find. Whether you can:

- **classify** a weakness into the right 2025 category — or record a CWE when no
  category fits, which for the memory findings is all of them;
- **dispose** of a tool finding with a reason someone else could check;
- **re-rate** a finding against what the code is actually for;
- notice what is **not there**.
