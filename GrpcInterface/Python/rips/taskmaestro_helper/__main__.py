"""Command-line entry point: `python -m rips.taskmaestro_helper <subcommand>`."""

from __future__ import annotations

import sys


def main() -> int:
    if len(sys.argv) < 2:
        print(
            "usage: python -m rips.taskmaestro_helper <introspect|run> ...",
            file=sys.stderr,
        )
        return 2

    subcommand = sys.argv[1]
    rest = sys.argv[2:]

    if subcommand == "introspect":
        from . import introspect

        return introspect.main(rest)

    if subcommand == "run":
        from . import run

        return run.main(rest)

    print(f"unknown subcommand: {subcommand}", file=sys.stderr)
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
