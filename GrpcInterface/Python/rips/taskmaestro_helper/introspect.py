"""Introspect a taskmaestro workflow directory and emit a JSON schema.

The schema is consumed by ResInsight to auto-generate property-editor
fields for each task's config inputs. Only fields listed in the workflow's
`config_fields` are inspected, so models that also reference non-Pydantic
types (e.g. live `rips` objects passed via `depends_on`) do not break
introspection.
"""

from __future__ import annotations

import datetime
import json
import sys
from pathlib import Path
from types import UnionType
from typing import Any, Union, get_args, get_origin

from pydantic.fields import FieldInfo

from .refs import annotation_to_resinsight_type


_SCALAR_TYPE_MAP: dict[type, str] = {
    str: "string",
    bool: "boolean",
    int: "integer",
    float: "number",
}

# Annotations reported as JSON-schema "string" with an extra `format` marker.
_DATE_FORMAT_MAP: dict[type, str] = {
    datetime.date: "date",
    datetime.datetime: "date-time",
}


def _annotation_type(annotation: object) -> str:
    """Map a Pydantic field annotation to a JSON-schema-style type label."""
    if isinstance(annotation, type) and annotation in _SCALAR_TYPE_MAP:
        return _SCALAR_TYPE_MAP[annotation]

    # Optional[T] / T | None — peel the union and map the non-None arm
    origin = get_origin(annotation)
    if origin in (Union, UnionType):
        non_none = [a for a in get_args(annotation) if a is not type(None)]
        if len(non_none) == 1:
            return _annotation_type(non_none[0])

    if origin in (list, tuple, set, frozenset):
        return "array"

    if isinstance(annotation, type):
        # Recognised rips object types are reported as "object" with a separate marker.
        if annotation_to_resinsight_type(annotation) is not None:
            return "object"

    return "string"


def _field_schema(field_name: str, field_info: FieldInfo) -> dict[str, Any]:
    entry: dict[str, Any] = {
        "name": field_name,
        "type": _annotation_type(field_info.annotation),
        "required": field_info.is_required(),
    }
    if field_info.description:
        entry["description"] = field_info.description
    if not field_info.is_required():
        default = field_info.get_default(call_default_factory=False)
        if default is not None:
            if isinstance(default, (datetime.date, datetime.datetime)):
                default = default.isoformat()
            entry["default"] = default
    fmt = _DATE_FORMAT_MAP.get(field_info.annotation)
    if fmt is not None:
        entry["format"] = fmt
    ri_type = annotation_to_resinsight_type(field_info.annotation)
    if ri_type is not None:
        entry["resinsight_type"] = ri_type
    return entry


def collect_schema(workflow_dir: Path) -> dict[str, Any]:
    """Load the workflow at `workflow_dir` and produce its UI schema."""
    workflow_yaml = workflow_dir / "workflow.yaml"
    input_yaml = workflow_dir / "input.yaml"
    if not workflow_yaml.is_file():
        raise FileNotFoundError(f"Missing workflow.yaml in {workflow_dir}")

    # Workflow source files (e.g. pipeline.py) live next to workflow.yaml and
    # are referenced as plain dotted paths in workflow.yaml — make them importable.
    workflow_dir_str = str(workflow_dir)
    if workflow_dir_str not in sys.path:
        sys.path.insert(0, workflow_dir_str)

    from taskmaestro.task import get_input_type
    from taskmaestro.yaml_config import _load_workflow_only

    wf, _ = _load_workflow_only(
        workflow_yaml,
        input_yaml if input_yaml.is_file() else None,
    )

    tasks: list[dict[str, Any]] = []
    for task_name, task_cls in wf.topological_order():
        config_field_names = wf.get_config_fields(task_name)
        if not config_field_names:
            continue

        input_type = get_input_type(task_cls)
        config_fields: list[dict[str, Any]] = []
        for field_name in sorted(config_field_names):
            field_info = input_type.model_fields.get(field_name)
            if field_info is None:
                config_fields.append(
                    {
                        "name": field_name,
                        "type": "string",
                        "required": True,
                        "error": "field not found in input model",
                    }
                )
                continue
            config_fields.append(_field_schema(field_name, field_info))

        tasks.append({"name": task_name, "config_fields": config_fields})

    return {
        "name": wf.name,
        "description": "",
        "tasks": tasks,
    }


def main(argv: list[str]) -> int:
    if len(argv) != 1:
        print("usage: introspect <workflow_dir>", file=sys.stderr)
        return 2
    workflow_dir = Path(argv[0]).resolve()
    try:
        schema = collect_schema(workflow_dir)
    except Exception as exc:
        print(
            json.dumps({"error": str(exc), "type": type(exc).__name__}), file=sys.stderr
        )
        return 1
    json.dump(schema, sys.stdout, indent=2)
    sys.stdout.write("\n")
    return 0
