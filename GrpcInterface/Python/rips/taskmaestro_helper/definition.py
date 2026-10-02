"""Workflow definition subcommands (``load`` and ``save``).

The conversion between workflow folders and editable definitions is done by
``taskmaestro.definition``. This module adds ResInsight's schema annotations and
makes sure object references, which are only valid for one run, are never saved.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from taskmaestro import definition as tm_definition
from taskmaestro.exceptions import DefinitionError

from .refs import strip_refs
from .schema import ResInsightSchemaGenerator

DEFINITION_FORMAT = tm_definition.DEFINITION_FORMAT
WORKFLOW_FILE = tm_definition.WORKFLOW_FILE
INPUT_FILE = tm_definition.INPUT_FILE


def load_definition(workflow_dir: Path, *, describe: bool = False) -> dict[str, Any]:
    """Load ``workflow.yaml`` (and ``input.yaml``) from a workflow folder."""
    return tm_definition.load_definition(
        workflow_dir, describe=describe, schema_generator=ResInsightSchemaGenerator
    )


def definition_from_workflow(
    workflow: Any,
    registered_id: str | None,
    class_ids: dict[type[Any], str],
    *,
    describe: bool = False,
) -> dict[str, Any]:
    """Convert an in-memory workflow to a definition."""
    return tm_definition.definition_from_workflow(
        workflow,
        registered_id,
        class_ids,
        describe=describe,
        schema_generator=ResInsightSchemaGenerator,
    )


def describe_yaml(
    workflow_yaml: Path, input_yaml: Path | None = None
) -> tuple[dict[str, Any] | None, dict[str, Any] | None]:
    """Describe a YAML workflow. Returns ``(description, error)``."""
    return tm_definition.describe_yaml(
        workflow_yaml, input_yaml, ResInsightSchemaGenerator
    )


def write_definition(
    definition: dict[str, Any],
    out_dir: Path,
    *,
    workflow_only: bool = False,
    backup: bool = False,
) -> dict[str, Any]:
    """Write ``workflow.yaml`` (and ``input.yaml``) for a definition."""
    return tm_definition.write_definition(
        definition,
        out_dir,
        workflow_only=workflow_only,
        backup=backup,
        value_filter=strip_refs,
    )


# --- Command-line entry points ---


def load_main(argv: list[str]) -> dict[str, Any]:
    parser = argparse.ArgumentParser(prog="load", description="Load a workflow folder.")
    parser.add_argument("workflow_dir", type=Path)
    parser.add_argument("--describe", action="store_true")
    args = parser.parse_args(argv)
    result = load_definition(args.workflow_dir, describe=args.describe)
    result["status"] = "ok"
    return result


def save_main(argv: list[str], stdin_text: str) -> dict[str, Any]:
    parser = argparse.ArgumentParser(
        prog="save",
        description="Write a workflow definition (read from stdin) as YAML.",
    )
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--workflow-only", action="store_true")
    parser.add_argument("--describe", action="store_true")
    parser.add_argument("--backup", action="store_true")
    args = parser.parse_args(argv)
    try:
        definition = json.loads(stdin_text)
    except json.JSONDecodeError as exc:
        raise DefinitionError(f"Invalid definition JSON: {exc}") from exc
    if not isinstance(definition, dict):
        raise DefinitionError("The definition must be a JSON object")

    result = write_definition(
        definition,
        args.out_dir,
        workflow_only=args.workflow_only,
        backup=args.backup,
    )
    if args.describe:
        input_path = Path(result["input_yaml"]) if result["input_yaml"] else None
        result["describe"], result["describe_error"] = describe_yaml(
            Path(result["workflow_yaml"]), input_path
        )
    return result
