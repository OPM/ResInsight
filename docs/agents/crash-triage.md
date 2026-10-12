# Crash triage

Investigate a **batch of 4–5 crash signatures** from the `resinsight-system-doc`
stacktrace registry and drive each to one of two outcomes:

- **A verified fix** → included in one shared fix PR pushed to your personal
  fork, with the signature's call stack in the PR body.
- **No confident fix** → record a note on the signature explaining the findings
  and why a fix is uncertain. Nothing is filed for it.

No GitHub issue is created; the call stacks live in the PR body and
`registry.json` holds the state. This is **step 3** of the weekly pipeline
(steps 1–2, dedupe + issue-linking, are done by `registry.py` /
`link_issues.py` in the system-doc repo).

> A local skill mirroring this doc is at `.claude/skills/crash-triage/SKILL.md`
> (gitignored). This file is the canonical version.

## Locations

- Registry repo (state + tooling): `resinsight-system-doc/stacktrace-reports/`
  - `registry.json` — **single source of truth**, one entry per signature
  - `registry.py worklist` — candidate signatures, highest impact first
  - `registry.py set --id <sid> ...` — write the outcome back
- Source tree to patch: this repo
- See [build.md](build.md) for build commands and [coding-style.md](coding-style.md)
  for the style the patch must match.

## Hard rules

- **`registry.json` status is the source of truth.** Only triage `new` or
  `linked` signatures, or `resolved` ones labelled **regressed**; update the
  status at every transition.
- **Do not create GitHub issues.** One PR covers the batch.
- **Never post to GitHub before the human gate.** Prepare everything, stop for
  approval, then run `gh`/`git push`.
- Branches and PRs go to your **personal fork**, never `origin` (OPM upstream).
- Commit messages: short title, fix description and call stack; no issue
  number, no AI attribution. No `## Test plan` section in the PR.
- Keep all comments and descriptions short and concise.
- **Never update previous week reports.** Run `registry.py render --date
  <latest-week>` only, never `render --all`.

## Status lifecycle

| status | meaning |
| --- | --- |
| `new` / `linked` | not yet triaged — eligible to pick |
| `investigating` | picked into the current batch |
| `pr-open` | fix is in the batch PR, awaiting merge |
| `resolved` | the batch PR has been **merged** |
| `no-fix-found` | investigated, no confident fix; note recorded |
| `on-hold` | deliberately parked |

### Regressions

`regressed` is a computed label, not a status. When a fix merges,
`link_issues.py` records `fixed_in` (the version in `ResInsightVersion.cmake`
at the merge commit). If the signature later crashes on a **strictly newer**
version, the weekly report shows `**Status:** resolved · **regressed**` plus a
`**Regressed:**` line, and the worklist lists it as `[resolved, regressed]`.

- Equal versions are not a regression.
- Newer *releases* only count if their `v<version>` tag contains the fix
  (cached in `fix_shipped`); dev/RC builds are compared by version only.
- `set --pr` clears `fixed_in`; the new merge sets it again. Override with
  `set --fixed-in VER` (or `""` to clear).

## Procedure

### 1. Pick the batch

Run `registry.py worklist` and take the 4–5 highest-impact entries
(`(unsymbolized crash site)` is already filtered out). It ranks unhandled
signatures by occurrences on the newest release, plus regressed ones. Still
check each candidate's status in `registry.json`.

For a regressed signature, first read the previous fix (`pr`) and the
`**Regressed:**` versions: was the old fix incomplete, lost, or do the new
crashes take a different path? Reference the previous PR in the new one.

Read each entry (`top_frame`, `signature_frames`, `representative_stack`,
`total_count`, `weeks`) and mark it picked:

```
python registry.py set --id <sid> --status investigating
```

Then work steps 2–5 per signature.

### 2. Locate the real crash site

Map each frame in `representative_stack` to source via its `at <path>:<line>`.
The reported line is often the optimizer's marker, not the true fault — read
the surrounding function and find the actual unsafe operation (null
dereference, out-of-range index, use-after-free, unchecked cast). Model case:
PR #14194, where the trace blamed `cvf::ref<…>::isNull()` but the real bug was
an unchecked `mainGrid()` returning `nullptr`. Use `git blame` for intent.

### 3. Form a fix

Write the **minimal** patch that removes the fault, matching the surrounding
code style and preferring a guard pattern that already exists in the same file.
A fix is only "verified" when (a) the root cause is sound and the patch provably
prevents the unsafe operation on the reported path, (b) it builds, **and**
(c) the reproducing test from step 4 passes.

If the root cause is a fragile call pattern rather than a one-off mistake, grep
the codebase for other occurrences and list them in the PR with a suggested fix
(example: the manual `resolveReferencesRecursively()` +
`initAfterReadRecursively()` pair combined into `initAfterInsert()`,
https://github.com/OPM/ResInsight/issues/14372).

Always check the close surroundings for the same defect — the rest of the
file/class and the consumers of the fixed function (callers repeating the
unsafe call or mishandling a new null/empty result) — and fix it in the same
commit. Keep these small; list anything larger in the PR instead of patching it.

### 4. Create a test reproducing the issue

Write a unit test that drives the crashing function with the triggering state
(null pointer, empty collection, missing result, ...), next to the existing
tests for that code (see [build.md](build.md)). Confirm it fails **without**
the patch and passes **with** it. If the path is unreachable from a test (GUI
state, external data), say so at the human gate and fall back to reasoning +
build verification.

### 5. Build-verify

Build the affected target (never call ninja directly):

```
cmake --build build --target <target owning the file>
```

A verified fix needs sound root-cause reasoning, a clean build, and the passing
test from step 4 (or a stated reason why none is feasible).

### 6. Human gate

Once the whole batch is investigated, present in one message — per signature:
the root cause, the patch diff (including surrounding fixes), the test and build results, and a confidence
rating — plus the draft PR body covering the batch. Then **stop** and ask the
user to approve, revise, or downgrade individual signatures to a note.

### 7a. On approval — one branch, one PR for the batch

Branch on the fork (`crash-triage-<YYYY-MM-DD>`), push to your personal fork,
and open a single PR with a short title.

One commit per signature: short title, short fix description, then the raw
call stack:

```
<short title>

<short description of the fix>

Call stack:
[0] ...
```

The PR body has one section per signature, so it contains the call stack of
**every** fix:

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

For every signature in a merged PR:

```
python registry.py set --id <sid> --pr-state MERGED --status resolved
python registry.py render --date <latest-week>
```

Signatures whose PR is still open stay at `pr-open`. `link_issues.py` fills
`fixed_in` on its next run (the regression baseline).

### 9. Commit the registry change

In the system-doc repo, commit `registry.json` (and any re-rendered report) on a
branch and push to the user's fork. Do this after step 7 and again after step 8.

## Notes

- The same `top_frame` can appear under two signature ids (different deeper
  paths). Fixing one does not auto-resolve the other — handle each, though the
  same PR may cover both; set the status on both ids.
- A regressed signature keeps its old `pr` until a new one is recorded with
  `set --pr`; picking it with `--status investigating` removes it from the
  worklist.
