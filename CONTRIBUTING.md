# Contributing to ResInsight

## What an open issue means

An open issue is either

- a confirmed, reproducible bug, or
- work the team realistically intends to do within about 18 months.

Everything else is closed with a reason. Closing is not deleting: the issue stays searchable, links keep working, and anyone can reopen it with new information.

Issues are closed when they are

- **stale**: no activity for about three years. These are closed as *not planned* and labelled `ClosedStale`.
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

## Pull requests

- Base pull requests on the `dev` branch.
- Format your code before pushing. CI checks formatting but does not fix it for you.

## Code formatting

The `clang-format` and `Python Linting` GitHub workflows run on every push (and, for Python, on
pull requests). They fail if any file is not formatted, and the job log shows the required diff.
They do **not** open fix pull requests automatically.

Format locally before you commit:

- **C++** (clang-format 19, configuration in `.clang-format`):

  ```bash
  clang-format-19 -i <changed files>
  ```

- **Python** (ruff, run from `GrpcInterface`):

  ```bash
  ruff check --fix .
  ruff format .
  ```

If you want CI to create a pull request with the fixes, start the workflow manually. In the GitHub
UI, open **Actions**, select **clang-format** or **Python Linting**, click **Run workflow**, pick
your branch and leave **Create a PR** selected. You can also use the GitHub CLI:

```bash
gh workflow run clang-format.yml --ref <branch> -f create_pr=true
gh workflow run python-linting.yml --ref <branch> -f create_pr=true
```