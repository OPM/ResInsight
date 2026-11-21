"""Command-line entry point: `python -m rips.mcp_server [--launch]`.

Serves MCP over stdio. ResInsight is looked up on the first tool call, so the
server can be started before ResInsight. While the server runs, the MCP stdio
transport points fd 1 at stderr, so the output of a ResInsight started with
--launch does not reach the MCP messages. That ResInsight is closed when the
server exits.
"""

from __future__ import annotations

import argparse
import threading

import grpc

import rips

from .server import create_server


class _Connection:
    """Finds a running ResInsight, or launches one, and keeps the instance."""

    def __init__(self, launch: bool) -> None:
        self._launch = launch
        self._instance: rips.Instance | None = None
        # Tools run on worker threads, so calls can arrive at the same time
        self._lock = threading.Lock()

    def __call__(self) -> rips.Instance:
        with self._lock:
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

    def close(self) -> None:
        """Exit the ResInsight this server launched, if any."""
        with self._lock:
            if self._instance is not None and self._instance.launched:
                try:
                    self._instance.exit()  # type: ignore[no-untyped-call]
                except (rips.RipsError, grpc.RpcError):
                    pass
                self._instance = None


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        prog="python -m rips.mcp_server",
        description="MCP server (stdio) for a running ResInsight.",
    )
    parser.add_argument(
        "--launch",
        action="store_true",
        help="launch ResInsight from RESINSIGHT_EXECUTABLE when no running"
        " instance is found, and close it when the server exits",
    )
    args = parser.parse_args(argv)
    connection = _Connection(args.launch)
    try:
        create_server(connection).run("stdio")
    finally:
        connection.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
