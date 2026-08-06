# Crash triage

Investigate a **batch of 4–5 crash signatures** from the `resinsight-system-doc`
stacktrace registry and drive each to one of two outcomes:

- **A verified fix** → included in one shared fix PR pushed to your personal
  fork, with the signature's call stack in the PR body.
- **No confident fix** → record a note on the signature explaining the findings
  and why a fix is uncertain. Nothing is filed for it.

No GitHub issue is created. The call stack that motivated each fix lives in the
PR body, and `registry.json` holds the state.

This is **step 3** of the weekly pipeline. Steps 1–2 (dedupe + issue-linking)
are done by `registry.py` / `link_issues.py` in the system-doc repo.

> A locally-invokable Claude Code skill mirroring this doc lives at
> `.claude/skills/crash-triage/SKILL.md` (untracked — `.claude/` is gitignored).
> This file is the canonical, checked-in version.

## Locations

- Registry repo (state + tooling): `resinsight-system-doc/stacktrace-reports/`
  - `registry.json` — **single source of truth**, one entry per signature
  - `registry.py worklist` — candidate signatures, highest impact first
  - `registry.py set --id <sid> ...` — write the outcome back
- Source tree to patch: this repo
- See [build.md](build.md) for build commands and [coding-style.md](coding-style.md)
  for the style the patch must match.

## Hard rules

- **`registry.json` status is the single source of truth** for what has already
  been handled. Never re-triage a signature whose status is anything other than
  `new` or `linked`, and update the status at every transition.
- **Do not create GitHub issues.** One PR covers the whole batch; each fix's
  call stack goes in the PR body.
- **Never post to GitHub before the human gate.** Prepare everything (patches,
  PR text) and stop for explicit approval. Only after approval run the
  `gh`/`git push` commands.
- Branches and PRs go to your **personal fork** remote, never `origin` (OPM upstream).
- Commit messages are a short description with no issue number (there is no
  issue); no AI attribution; no `## Test plan` section in the PR.
- Keep all comments and descriptions (PR, notes, code) short and concise.

## Status lifecycle

| status | meaning |
| --- | --- |
| `new` / `linked` | not yet triaged — eligible to pick |
| `investigating` | picked into the current batch |
| `pr-open` | fix is in the batch PR, awaiting merge |
| `resolved` | the batch PR has been **merged** |
| `no-fix-found` | investigated, no confident fix; note recorded |
| `on-hold` | deliberately parked |

## Procedure

### 1. Pick the batch

Run `registry.py worklist` and take the 4–5 highest-impact entries. The
`(unsymbolized crash site)` bucket is already filtered out — those stacks have
no ResInsight symbol at the fault and are not individually actionable.

`worklist` does not filter on status, so **check each candidate's status in
`registry.json` and skip anything that is not `new` or `linked`** — otherwise
signatures already covered by an open PR resurface.

Read each entry (`top_frame`, `signature_frames`, `representative_stack`,
`total_count`, `weeks`) and mark it picked:

```
python registry.py set --id <sid> --status investigating
```

Then work steps 2–5 per signature.

### 2. Locate the real crash site

Map each frame in `representative_stack` to source via its `at <path>:<line>`.
The reported crash line is often the optimizer's line marker, not the true
fault — read the surrounding function and find the actual unsafe operation
(null dereference, out-of-range index, use-after-free, unchecked cast). The
model investigation is PR #14194: the trace blamed `cvf::ref<…>::isNull()` but
the real bug was an unchecked `mainGrid()` returning `nullptr`. Use
`git log`/`git blame` on the crash site to understand intent.

### 3. Form a fix

Write the **minimal** patch that removes the fault, matching the surrounding
code style and preferring a guard pattern that already exists in the same file.
A fix is only "verified" when (a) the root cause is sound and the patch provably
prevents the unsafe operation on the reported path, (b) it builds, **and**
(c) the reproducing test from step 4 passes.

If the root cause is a fragile call pattern rather than a one-off mistake, grep
the codebase for other occurrences of the same pattern and list them in the PR
with a suggested fix for all locations (example: the manual
`resolveReferencesRecursively()` + `initAfterReadRecursively()` pair combined
into `initAfterInsert()`, https://github.com/OPM/ResInsight/issues/14372).
Still patch only the crashing site unless told otherwise.

### 4. Create a test reproducing the issue

Write a unit test that reproduces the crash path: drive the crashing function
with the state that triggers the fault (null pointer, empty collection, missing
result, ...). Place it alongside the existing tests for the affected code (see
[build.md](build.md) for test locations and commands). Confirm the test fails
or crashes **without** the patch, then passes **with** it. If the crash path
cannot be reached from a test (e.g. it requires GUI state or external data),
say so at the human gate and fall back to reasoning + build verification.

### 5. Build-verify

Build the affected target with CMake (never call ninja directly):

```
cmake --build build --target <target owning the file>
```

If it does not compile, fix and rebuild. The bar for a verified fix is sound
root-cause reasoning, a clean build, and the reproducing test from step 4
passing (or a stated reason why no test is feasible).

### 6. Human gate

Once the whole batch is investigated, present in one message — per signature:
the root cause, the patch diff, the test and build results, and a confidence
rating — plus the draft PR body covering the batch. Then **stop** and ask the
user to approve, revise, or downgrade individual signatures to a note.

### 7a. On approval — one branch, one PR for the batch

Branch on the fork (`crash-triage-<YYYY-MM-DD>`), one commit per signature with
a short description, push to your personal fork, and open a single PR with a
short title.

The PR body has one section per signature containing a one-line Problem/Fix and
the raw `representative_stack` in a code fence (this replaces the issue that
used to carry it):

````markdown
### <short description>

Problem: <one line>
Fix: <one line>

<details><summary>Crash stack (signature <sid>, <total_count> reports)</summary>

```
[0] ...
[1] ...
```

</details>
````

Then record the outcome for **every** signature in the batch:

```
python registry.py set --id <sid> --pr <pr> --branch crash-triage-<date> --status pr-open
python registry.py render --date <latest-week>
```

### 7b. On uncertainty — record a note, file nothing

```
python registry.py set --id <sid> --status no-fix-found \
  --note "Crash at <site>. Findings: <ruled in/out>. Uncertain because <reason>."
```

### 8. When the PR is merged — mark resolved

The PR merging is what closes a signature out. For every signature in that PR:

```
python registry.py set --id <sid> --status resolved
python registry.py render --date <latest-week>
```

Signatures whose PR is still open stay at `pr-open`; do not mark them resolved
early.

### 9. Commit the registry change

In the system-doc repo, commit `registry.json` (and any re-rendered report) on a
branch and push to the user's fork. Do this after step 7 and again after step 8.

## Notes

- The same `top_frame` can appear under two signature ids (different deeper
  paths). Fixing one does not auto-resolve the other — handle each, though the
  same PR may cover both; set the status on both ids.
