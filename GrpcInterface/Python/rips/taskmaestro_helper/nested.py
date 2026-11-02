"""The ``describe-nested`` subcommand: describe a nested workflow, read-only.

A nested workflow is a task that wraps a whole workflow, from a ``workflow:``
entry in YAML or ``Workflow.as_task()`` in a registered workflow. ResInsight
shows it in its own tab, drawn from the description.
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Any

import yaml
from taskmaestro.exceptions import DefinitionError

from .schema import ResInsightSchemaGenerator


def nested_entries(workflow_yaml: Path) -> dict[str, dict[str, Any]]:
    """The ``workflow:`` entries of a YAML workflow, by task name.

    Paths are as written in the file, relative to its folder. A best effort:
    errors are reported by the description instead.
    """
    try:
        raw = yaml.safe_load(workflow_yaml.read_text())
        tasks = raw["workflow"]["tasks"]
    except Exception:  # noqa: BLE001
        return {}

    entries: dict[str, dict[str, Any]] = {}
    for task in tasks if isinstance(tasks, list) else []:
        if not isinstance(task, dict) or not isinstance(task.get("workflow"), str):
            continue
        name = task.get("name") or _inner_workflow_name(
            workflow_yaml.parent / task["workflow"]
        )
        entries[str(name)] = {
            "workflow": task["workflow"],
            "workflow_input": task.get("workflow_input"),
        }
    return entries


def _inner_workflow_name(workflow_yaml: Path) -> str:
    # The task is named after the inner workflow when the entry has no name
    try:
        return str(yaml.safe_load(workflow_yaml.read_text())["workflow"]["name"])
    except Exception:  # noqa: BLE001
        return workflow_yaml.stem


def describe_nested_yaml(
    workflow_yaml: Path, input_yaml: Path | None, import_dirs: list[Path]
) -> dict[str, Any]:
    """Describe the inner YAML file of a ``workflow:`` entry."""
    from taskmaestro import definition as tm_definition
    from taskmaestro.yaml_config import workflow_imports

    if not workflow_yaml.is_file():
        raise DefinitionError(f"Cannot find the nested workflow '{workflow_yaml}'")

    # Task modules of the outer workflow folders must be importable too, as when
    # the outer workflow loads the inner one
    def describe(dirs: list[Path]) -> tuple[Any, Any]:
        if not dirs:
            return tm_definition.describe_yaml(
                workflow_yaml, input_yaml, ResInsightSchemaGenerator
            )
        with workflow_imports(dirs[0]):
            return describe(dirs[1:])

    description, error = describe(import_dirs)
    name = description["workflow"] if description else workflow_yaml.stem
    return {
        "name": name,
        "describe": description,
        "describe_error": error,
        "nested": nested_entries(workflow_yaml),
        "source": {"workflow_yaml": str(workflow_yaml.resolve())},
    }


def describe_nested_registered(
    workflow_id: str, task_path: list[str]
) -> dict[str, Any]:
    """Describe a workflow wrapped as a task in a registered workflow.

    ``task_path`` is the name of the nested task, followed by the names of
    nested tasks inside it for deeper levels.
    """
    from taskmaestro.discovery import get_registered_workflow
    from taskmaestro.exceptions import PluginLoadError
    from taskmaestro.introspection import describe_workflow

    try:
        workflow = get_registered_workflow(workflow_id)
    except PluginLoadError as exc:
        raise DefinitionError(str(exc)) from exc

    for task_name in task_path:
        tasks = dict(workflow.topological_order())
        if task_name not in tasks:
            raise DefinitionError(
                f"Workflow '{workflow.name}' has no task '{task_name}'"
            )
        # Set by taskmaestro's workflow_task(), which wraps a workflow as a task
        inner = getattr(tasks[task_name], "_inner_workflow", None)
        if inner is None:
            raise DefinitionError(f"Task '{task_name}' is not a nested workflow")
        workflow = inner

    return {
        "name": workflow.name,
        "describe": describe_workflow(
            workflow, schema_generator=ResInsightSchemaGenerator
        ),
        "describe_error": None,
        "nested": {},
        "source": {"registered_id": workflow_id, "task_path": task_path},
    }


def describe_nested_main(argv: list[str]) -> dict[str, Any]:
    parser = argparse.ArgumentParser(
        prog="describe-nested", description="Describe a nested workflow."
    )
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--yaml", type=Path, help="The inner workflow file")
    source.add_argument("--registered", help="The id of the registered workflow")
    parser.add_argument("--input", type=Path, help="The workflow_input of the entry")
    parser.add_argument(
        "--import-dir",
        type=Path,
        action="append",
        default=[],
        help="Folder with task modules (the folders of the outer workflows)",
    )
    parser.add_argument(
        "--task",
        action="append",
        default=[],
        help="Nested task of a registered workflow, repeated for deeper levels",
    )
    args = parser.parse_args(argv)

    if args.yaml is not None:
        result = describe_nested_yaml(args.yaml, args.input, args.import_dir)
    else:
        if not args.task:
            parser.error("--registered requires --task")
        result = describe_nested_registered(args.registered, args.task)
    result["status"] = "ok"
    return result
