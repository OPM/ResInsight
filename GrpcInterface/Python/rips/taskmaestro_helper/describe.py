"""Workflow descriptions in the shape of ``taskmaestro workflow describe --json``.

``describe_yaml`` runs taskmaestro's own describe implementation in-process so
ResInsight gets the authoritative validation result for a YAML workflow, but
with the real error message (the public CLI deliberately hides it).
``describe_workflow`` produces the same document for an in-memory (registered)
workflow, using only the public ``Workflow`` API.
"""

from __future__ import annotations

import argparse
import io
import json
from contextlib import redirect_stdout
from dataclasses import asdict
from pathlib import Path
from typing import Any

from .schema import model_json_schema, python_type_name


def _error_from_exception(exc: Exception) -> dict[str, Any]:
    from taskmaestro.cli import _error

    return _error("configuration_error", exc, str(exc))


def describe_yaml(
    workflow_yaml: Path, input_yaml: Path | None = None
) -> tuple[dict[str, Any] | None, dict[str, Any] | None]:
    """Describe a YAML workflow. Returns ``(description, error)``."""
    from taskmaestro import cli
    from taskmaestro.exceptions import ConfigLoadError, PluginLoadError

    args = argparse.Namespace(
        workflow=str(workflow_yaml),
        input=str(input_yaml) if input_yaml is not None else None,
        json=True,
    )
    captured = io.StringIO()
    try:
        with redirect_stdout(captured):
            cli._workflow_describe(args)
    except (ConfigLoadError, PluginLoadError) as exc:
        return None, _error_from_exception(exc)
    except Exception as exc:  # Errors raised by task modules at import time.
        return None, _error_from_exception(exc)
    try:
        return json.loads(captured.getvalue()), None
    except json.JSONDecodeError as exc:
        return None, {
            "code": "internal_error",
            "message": f"Invalid describe output: {exc}",
        }


def _output_ref(ref: Any) -> dict[str, Any]:
    return {"task": ref.task_name, "field": ref.output_field}


def dependency_spec(deps: Any) -> Any:
    """Render dependencies the same way ``taskmaestro workflow describe`` does."""
    from taskmaestro.dependencies import CollectionRef

    if deps is None:
        return None
    if isinstance(deps, str):
        return {"task": deps, "field": None}
    if isinstance(deps, tuple):
        return {"task": deps[0], "field": deps[1]}
    result: dict[str, Any] = {}
    for field, ref in deps.items():
        if isinstance(ref, CollectionRef):
            members: list[Any] | dict[str, Any]
            if ref.kind == "keyed":
                members = {key: _output_ref(item) for key, item in ref.keyed_members}
            else:
                members = [_output_ref(item) for item in ref.positional_members]
            result[field] = {"collect": {"kind": ref.kind, "members": members}}
        elif isinstance(ref, tuple):
            result[field] = {"task": ref[0], "field": ref[1]}
        else:
            result[field] = {"task": ref, "field": None}
    return result


def describe_workflow(workflow: Any) -> dict[str, Any]:
    """Describe an in-memory workflow without input values."""
    from taskmaestro.task import get_input_type

    tasks: list[dict[str, Any]] = []
    for name, task in workflow.topological_order():
        input_type = get_input_type(task)
        output_type = workflow.get_output_annotation(name)
        task_map = workflow.get_task_map(name)
        tasks.append(
            {
                "name": name,
                "python_type": python_type_name(task),
                "depends_on": dependency_spec(workflow.get_dependencies(name)),
                "config_fields": sorted(workflow.get_config_fields(name)),
                "provided_config_fields": None,
                "missing_config_fields": None,
                "config_values": None,
                "required_input_fields": sorted(
                    field
                    for field, info in input_type.model_fields.items()
                    if info.is_required()
                ),
                "map": asdict(task_map) if task_map is not None else None,
                "input_schema": model_json_schema(input_type),
                "output_schema": model_json_schema(output_type),
            }
        )
    return {
        "workflow": workflow.name,
        "result_task": workflow.result_task_name,
        "tasks": tasks,
    }
