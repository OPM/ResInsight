# Contributing to ResInsight

## What an open issue means

An open issue is either

- a confirmed, reproducible bug, or
- work the team realistically intends to do within about 18 months.

Everything else is closed with a reason. Closing is not deleting: the issue stays searchable, links keep working, and anyone can reopen it with new information.

Issues are closed when they are

- **stale**: no activity for about three years. These are closed as *not planned* and labelled `closed-stale`.
- **obsolete or fixed**: they refer to code, file formats or UI that has since been rewritten or fixed.
- **waiting on the reporter**: labelled `NeedsInput` and closed if there is no answer within 30 days.
- **duplicates** or **not reproducible** (`Cannot Reproduce`).
- **covered by a tracking issue**: related issues are consolidated into one issue, and the rest are closed as duplicates of it.

If an issue was closed and still matters in the current release, reopen it or comment with details.

## Reporting a bug

Search existing issues first, including closed ones. Include:

- ResInsight version
- Operating system
- Steps to reproduce, and what you expected to happen
- Sample data or a project file, if you can share it

## Ideas and feature requests

Ideas without a concrete plan belong in [Discussions](https://github.com/OPM/ResInsight/discussions), where they can be discussed and upvoted. Ideas that get picked up are moved to an issue.

## Pull requests

- Base pull requests on the `dev` branch.
- Format C++ changes with clang-format, and follow [docs/agents/coding-style.md](docs/agents/coding-style.md).