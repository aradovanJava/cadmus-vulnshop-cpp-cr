# CR1 findings — <your name>

Date: ____________  Environment: VM1 (Windows 11)

---

## Task 1 — Orientation

| Question | Answer |
|---|---|
| Distinct HTTP entry points found by browsing | |
| Pages whose behaviour depends on who is signed in | |
| Page that displays something another user could have written | |

---

## Task 2 — Static analysis findings triaged

Verdicts: `TP` · `FP-scope` · `FP-context` · `DIS` · `NOT-SEC`

| # | Rule | File:line | Tool severity | Verdict | My severity | Why they differ |
|---|---|---|---|---|---|---|
| 1 | | | | | | |
| 2 | | | | | | |
| 3 | | | | | | |
| 4 | | | | | | |
| 5 | | | | | | |
| 6 | | | | | | |
| 7 | | | | | | |
| 8 | | | | | | |
| 9 | | | | | | |
| 10 | | | | | | |

**Findings I re-rated upward, and why:**

>

---

## Task 3 — Composition analysis

| | Count |
|---|---|
| Dependencies declared in `vcpkg.json` | |
| Directories under `third_party/` | |
| Components Snyk reports testing | |

**What the disagreement between those three numbers means:**

>

| Component | Advisory | Severity | Reachable? | Evidence (how you checked) |
|---|---|---|---|---|
| | | | | |
| | | | | |

**My remediation order:** ____________________________________________

**It differs from severity order because:**

>

---

## Task 4 — Dynamic analysis

| # | Diagnostic | File:line | Which request reached it | CWE |
|---|---|---|---|---|
| 1 | | | | |
| 2 | | | | |
| 3 | | | | |
| 4 | | | | |
| 5 | | | | |

**Did any of these also appear in the static analysis? Which, and at what severity?**

>

---

## Task 5 — Weaknesses found by inspection only

| # | Weakness | File | OWASP 2025 or CWE | How you found it |
|---|---|---|---|---|
| 1 | | | | |
| 2 | | | | |
| 3 | | | | |
| 4 | | | | |
| 5 | | | | |
| 6 | | | | |

**Absences I looked for and did not find:**

>

---

## One finding, written up in full

| Field | |
|---|---|
| Title (the class, not the symptom) | |
| Location (file, line, commit) | |
| Category / CWE | |
| Verdict and evidence | |
| **Reachability** — how the input gets there | |
| Root cause, stated as a decision | |
| Remediation and pattern | |
| **Proof** — the test or tool run that would show it fixed | |

---

## Tally

| | Count |
|---|---|
| Weaknesses the tools found | |
| Weaknesses I found that the tools did not | |

**The one finding I would escalate first, and why:**

>
