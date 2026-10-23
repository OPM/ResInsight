"""Command-line entry point: `python -m rips.taskmaestro_helper <subcommand>`.

Subcommands other than ``run`` print exactly one JSON document to stdout. Any
output written while importing task modules is redirected to stderr, including
output written by extension modules and child processes straight to file
descriptor 1. On
failure they exit with code 2 and print
``{"status": "invalid", "error": {"message", "task", "field"}}``.
"""

from __future__ import annotations

import json
import os
import sys
from collections.abc import Callable, Iterator
from contextlib import contextmanager, redirect_stdout
from typing import Any

USAGE = (
    "usage: python -m rips.taskmaestro_helper "
    "{catalog,load,save,export-registered,run} ..."
)


def _catalog(argv: list[str]) -> dict[str, Any]:
    from .catalog import catalog_main

    return catalog_main(argv)


def _load(argv: list[str]) -> dict[str, Any]:
    from .definition import load_main

    return load_main(argv)


def _save(argv: list[str]) -> dict[str, Any]:
    from .definition import save_main

    return save_main(argv, sys.stdin.read())


def _export_registered(argv: list[str]) -> dict[str, Any]:
    from .catalog import export_registered_main

    return export_registered_main(argv)


JSON_COMMANDS: dict[str, Callable[[list[str]], dict[str, Any]]] = {
    "catalog": _catalog,
    "load": _load,
    "save": _save,
    "export-registered": _export_registered,
}


def _invalid(error: dict[str, Any]) -> str:
    return json.dumps({"status": "invalid", "error": error})


def _error_json(exc: Exception) -> dict[str, Any]:
    to_json = getattr(exc, "to_json", None)  # taskmaestro's DefinitionError
    if callable(to_json):
        return dict(to_json())
    if isinstance(exc, ImportError) and (exc.name or "").startswith("taskmaestro"):
        message = f"taskmaestro>=0.4,<0.5 is required to edit workflows: {exc}"
    else:
        message = f"{type(exc).__name__}: {exc}"
    return {"message": message, "task": None, "field": None}


@contextmanager
def _stdout_to_stderr() -> Iterator[None]:
    """Send everything written to stdout to stderr, at the file descriptor level."""
    sys.stdout.flush()
    try:
        saved_fd = os.dup(1)
    except OSError:  # no usable stdout file descriptor; redirect Python output only
        saved_fd = None
    try:
        if saved_fd is not None:
            os.dup2(2, 1)
        with redirect_stdout(sys.stderr):
            yield
    finally:
        sys.stderr.flush()
        if saved_fd is not None:
            os.dup2(saved_fd, 1)
            os.close(saved_fd)


def run_json_command(
    command: Callable[[list[str]], dict[str, Any]], argv: list[str]
) -> int:
    stdout = sys.stdout
    try:
        with _stdout_to_stderr():
            result = command(argv)
    except SystemExit as exc:  # argparse errors
        print(
            _invalid(
                {"message": f"Invalid arguments: {exc}", "task": None, "field": None}
            ),
            file=stdout,
        )
        return 2
    except Exception as exc:  # noqa: BLE001 - always answer ResInsight with JSON
        print(_invalid(_error_json(exc)), file=stdout)
        return 2
    print(json.dumps(result), file=stdout)
    return 0


def main(argv: list[str] | None = None) -> int:
    args = sys.argv[1:] if argv is None else argv
    if not args:
        print(USAGE, file=sys.stderr)
        return 2

    subcommand, rest = args[0], args[1:]
    if subcommand == "run":
        from . import run

        return run.main(rest)

    command = JSON_COMMANDS.get(subcommand)
    if command is None:
        print(f"unknown subcommand: {subcommand}\n{USAGE}", file=sys.stderr)
        return 2
    return run_json_command(command, rest)


if __name__ == "__main__":
    raise SystemExit(main())
