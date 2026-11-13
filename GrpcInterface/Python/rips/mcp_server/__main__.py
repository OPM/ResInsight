"""Command-line entry point: `python -m rips.mcp_server [--launch]`.

Serves MCP over stdio. ResInsight is looked up on the first tool call, so the
server can be started before ResInsight.
"""

from __future__ import annotations

import argparse

import rips

from .server import create_server


class _Connection:
    """Finds a running ResInsight, or launches one, and keeps the instance."""

    def __init__(self, launch: bool) -> None:
        self._launch = launch
        self._instance: rips.Instance | None = None

    def __call__(self) -> rips.Instance:
        if self._instance is not None:
            try:
                self._instance.check_alive()
            except rips.RipsError:
                self._instance = None
        if self._instance is None:
            try:
                self._instance = rips.Instance.find()
            except rips.RipsError:
                if not self._launch:
                    raise
                self._instance = rips.Instance.launch()
        return self._instance


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        prog="python -m rips.mcp_server",
        description="MCP server (stdio) for a running ResInsight.",
    )
    parser.add_argument(
        "--launch",
        action="store_true",
        help="launch ResInsight from RESINSIGHT_EXECUTABLE when no running"
        " instance is found",
    )
    args = parser.parse_args(argv)
    create_server(_Connection(args.launch)).run("stdio")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
