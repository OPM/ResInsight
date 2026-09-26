#!/usr/bin/env python3
"""
Close open GitHub issues with no activity for a given number of years,
following the issue policy in CONTRIBUTING.md. Requires an authenticated gh CLI.

Lists the candidates by default. Pass --apply to label and close them.

Usage:
    python close_stale_issues.py [--years 3] [--keep-label KeepOpen] [--apply]
"""

import argparse
import json
import subprocess
import time
from datetime import datetime, timedelta, timezone

STALE_LABEL = "closed-stale"

COMMENT = (
    "Closing as part of a backlog cleanup: no activity for {years}+ years. "
    "See the issue policy in "
    "[CONTRIBUTING.md](https://github.com/{repo}/blob/dev/CONTRIBUTING.md).\n\n"
    "If this is still relevant in the current release, please reopen with details."
)


def gh(*args: str) -> str:
    return subprocess.run(
        ["gh", *args], check=True, capture_output=True, text=True, encoding="utf-8"
    ).stdout


def find_stale_issues(repo: str, cutoff: datetime, keep_labels: set) -> list:
    # Filter locally, as gh issue list --search is capped at 1000 results
    issues = json.loads(
        gh(
            "issue",
            "list",
            "-R",
            repo,
            "--state",
            "open",
            "--limit",
            "10000",
            "--json",
            "number,title,updatedAt,labels",
        )
    )

    stale = []
    for issue in issues:
        updated = datetime.fromisoformat(issue["updatedAt"].replace("Z", "+00:00"))
        labels = {label["name"] for label in issue["labels"]}
        if updated < cutoff and not labels & keep_labels:
            stale.append(issue)

    return sorted(stale, key=lambda issue: issue["updatedAt"])


def main():
    parser = argparse.ArgumentParser(description=__doc__.strip().splitlines()[0])
    parser.add_argument("--repo", default="OPM/ResInsight")
    parser.add_argument(
        "--years", type=float, default=3, help="Inactivity threshold (default: 3)"
    )
    parser.add_argument(
        "--keep-label",
        action="append",
        default=None,
        help="Never close issues with this label; can be repeated (default: KeepOpen)",
    )
    parser.add_argument(
        "--apply", action="store_true", help="Label and close the issues"
    )
    args = parser.parse_args()

    keep_labels = set(args.keep_label or ["KeepOpen"])
    cutoff = datetime.now(timezone.utc) - timedelta(days=365.25 * args.years)
    stale = find_stale_issues(args.repo, cutoff, keep_labels)

    for issue in stale:
        print(f"#{issue['number']:<6} {issue['updatedAt'][:10]}  {issue['title']}")
    print(f"\n{len(stale)} issues not updated since {cutoff:%Y-%m-%d}")

    if not args.apply or not stale:
        if stale:
            print("Dry run; pass --apply to close them.")
        return

    gh(
        "label",
        "create",
        STALE_LABEL,
        "-R",
        args.repo,
        "--force",
        "--description",
        "Closed by backlog cleanup due to inactivity",
    )

    comment = COMMENT.format(years=f"{args.years:g}", repo=args.repo)
    for issue in stale:
        number = str(issue["number"])
        gh("issue", "edit", number, "-R", args.repo, "--add-label", STALE_LABEL)
        gh(
            "issue",
            "close",
            number,
            "-R",
            args.repo,
            "--reason",
            "not planned",
            "--comment",
            comment,
        )
        print(f"Closed #{number}")
        # Stay below GitHub's secondary rate limit for content creation
        time.sleep(1)


if __name__ == "__main__":
    main()
