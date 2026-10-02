"""Command-line entry point: `python -m rips.taskmaestro_helper <subcommand>`.

Subcommands other than ``run`` print exactly one JSON document to stdout. Any
output written while importing task modules is redirected to stderr. On
failure they exit with code 2 and print
``{"status": "invalid", "error": {"message", "task", "field"}}``.
"""

from __future__ import annotations

import json
import sys
from collections.abc import Callable
from contextlib import redirect_stdout
from typing import Any

from ._compat import HelperError

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


def run_json_command(
    command: Callable[[list[str]], dict[str, Any]], argv: list[str]
) -> int:
    from ._compat import require_taskmaestro

    stdout = sys.stdout
    try:
        with redirect_stdout(sys.stderr):
            require_taskmaestro()
            result = command(argv)
    except HelperError as exc:
        print(_invalid(exc.to_json()), file=stdout)
        return 2
    except SystemExit as exc:  # argparse errors
        print(
            _invalid(
                {"message": f"Invalid arguments: {exc}", "task": None, "field": None}
            ),
            file=stdout,
        )
        return 2
    except Exception as exc:
        print(
            _invalid(
                {"message": f"{type(exc).__name__}: {exc}", "task": None, "field": None}
            ),
            file=stdout,
        )
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
