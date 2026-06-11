# Crash triage

Investigate one crash signature from the `resinsight-system-doc` stacktrace
registry and drive it to one of two outcomes:

- **A verified fix** → file an OPM issue, push a fix branch to your personal
  fork, open a PR, and record all three on the signature.
- **No confident fix** → record a note explaining the findings and why a fix is
  uncertain. No issue or PR is filed.

This is **step 3** of the weekly pipeline. Steps 1–2 (dedupe + issue-linking)
are done by `registry.py` / `link_issues.py` in the system-doc repo; only
signatures with **no linked issue** reach this stage.

> A locally-invokable Claude Code skill mirroring this doc lives at
> `.claude/skills/crash-triage/SKILL.md` (untracked — `.claude/` is gitignored).
> This file is the canonical, checked-in version.

## Locations

- Registry repo (state + tooling): `resinsight-system-doc/stacktrace-reports/`
  - `registry.json` — source of truth, one entry per signature
  - `registry.py worklist` — unlinked signatures, highest impact first
  - `registry.py set --id <sid> ...` — write the outcome back
- Source tree to patch: this repo
- See [build.md](build.md) for build commands and [coding-style.md](coding-style.md)
  for the style the patch must match.

## Hard rules

- **Never post to GitHub before the human gate.** Prepare everything (issue text,
  patch, PR text) and stop for explicit approval. Only after approval run the
  `gh`/`git push` commands.
- Branches and PRs go to your **personal fork** remote, never `origin` (OPM upstream).
- Commit/PR messages start with `#<issue>`; no AI attribution; no `## Test plan` section.
- One signature per run unless explicitly told to batch.

## Procedure

### 1. Pick the signature

If given a `signature_id`, use it. Otherwise run `registry.py worklist` and take
the highest-impact entry (the `(unsymbolized crash site)` bucket is already
filtered out — those stacks have no ResInsight symbol at the fault and are not
individually actionable). Read its registry entry (`top_frame`,
`signature_frames`, `representative_stack`, `total_count`, `weeks`) and set
status to `investigating`:

```
python registry.py set --id <sid> --status investigating
```

### 2. Locate the real crash site

Map each frame in `representative_stack` to source via its `at <path>:<line>`.
The reported crash line is often the optimizer's line marker, not the true
fault — read the surrounding function and find the actual unsafe operation
(null dereference, out-of-range index, use-after-free, unchecked cast). The
model investigation is PR #14194 / issue #14193: the trace blamed
`cvf::ref<…>::isNull()` but the real bug was an unchecked `mainGrid()` returning
`nullptr`. Use `git log`/`git blame` on the crash site to understand intent.

### 3. Form a fix

Write the **minimal** patch that removes the fault, matching the surrounding
code style and preferring a guard pattern that already exists in the same file.
A fix is only "verified" when (a) the root cause is sound and the patch provably
prevents the unsafe operation on the reported path, **and** (b) it builds.

### 4. Build-verify

Build the affected target with CMake (never call ninja directly):

```
cmake --build build --target <target owning the file>
```

If it does not compile, fix and rebuild. No runtime reproduction is required
(crashes here have no repro); sound reasoning + a clean build is the bar.

### 5. Human gate

Present, in one message: the root cause, the patch diff, the build result, a
confidence rating, and the drafts for the issue and PR. Then **stop** and ask
the user to approve, revise, or downgrade to a note.

### 6a. On approval — file issue, branch, PR

Create the issue (body = the raw stack in a code fence, matching #14193), with
labels `BugInRelease` and `PendingRelease`. Branch on the fork
(`fix-<issue>-<slug>`), commit `#<issue> <description>`, push to your personal fork, and
open the PR (title `#<issue> <description>`, body `Fixes #<issue>` + Problem/Fix
sections, matching #14194). Then record the outcome:

```
python registry.py set --id <sid> --issue <issue> --state OPEN \
  --pr <pr> --branch fix-<issue>-<slug> --status pr-open
python registry.py render --date <latest-week>
```

### 6b. On uncertainty — record a note, file nothing

```
python registry.py set --id <sid> --status no-fix-found \
  --note "Crash at <site>. Findings: <ruled in/out>. Uncertain because <reason>."
```

### 7. Commit the registry change

In the system-doc repo, commit `registry.json` (and any re-rendered report) on a
branch and push to the user's fork.

## Notes

- The same `top_frame` can appear under two signature ids (different deeper
  paths). Linking/fixing one does not auto-resolve the other — handle each,
  though the same issue/PR may cover both.
