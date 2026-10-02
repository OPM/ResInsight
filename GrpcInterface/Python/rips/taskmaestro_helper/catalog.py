"""Catalog of installed taskmaestro tasks and workflows.

Every entry point is loaded on its own, so one broken plugin is reported in
``errors`` while the rest of the catalog still loads.
"""

from __future__ import annotations

from typing import Any

import taskmaestro
from taskmaestro.exceptions import DefinitionError

from .definition import definition_from_workflow
from .schema import task_descriptor

CATALOG_FORMAT = 1


def _error(problem: Any) -> dict[str, Any]:
    return {
        "kind": problem.kind,
        "id": problem.name,
        "entry_point": problem.entry_point,
        "message": problem.message,
    }


def class_ids(tasks: dict[str, type[Any]]) -> dict[type[Any], str]:
    """Map task classes back to their (first) entry-point id."""
    result: dict[type[Any], str] = {}
    for task_id, cls in sorted(tasks.items()):
        result.setdefault(cls, task_id)
    return result


def _workflow_entry(
    workflow_id: str, workflow: Any, ids: dict[type[Any], str]
) -> dict[str, Any]:
    definition = definition_from_workflow(workflow, workflow_id, ids, describe=True)
    return {
        "id": workflow_id,
        "name": workflow.name,
        "definition": definition,
        "describe": definition["describe"],
        "editable": definition["editable"],
        "readonly_reasons": definition["readonly_reasons"],
    }


def build_catalog(*, include_workflows: bool = True) -> dict[str, Any]:
    from taskmaestro.discovery import load_registry

    registry = load_registry(include_workflows=include_workflows)
    errors = [_error(problem) for problem in registry.problems]
    descriptors: list[dict[str, Any]] = []
    for task_id, cls in sorted(registry.tasks.items()):
        try:
            descriptors.append(task_descriptor(task_id, cls))
        except Exception as exc:  # noqa: BLE001 - report plugin errors, keep the rest
            errors.append(
                {
                    "kind": "task",
                    "id": task_id,
                    "entry_point": "",
                    "message": f"Cannot describe task: {exc}",
                }
            )

    workflows: list[dict[str, Any]] = []
    ids = class_ids(registry.tasks)
    for workflow_id, workflow in sorted(registry.workflows.items()):
        try:
            workflows.append(_workflow_entry(workflow_id, workflow, ids))
        except Exception as exc:  # noqa: BLE001 - report plugin errors, keep the rest
            errors.append(
                {
                    "kind": "workflow",
                    "id": workflow_id,
                    "entry_point": "",
                    "message": str(exc),
                }
            )

    return {
        "format": CATALOG_FORMAT,
        "status": "ok",
        "taskmaestro_version": taskmaestro.__version__,
        "tasks": descriptors,
        "workflows": workflows,
        "errors": errors,
    }


def export_registered(workflow_id: str, *, describe: bool = False) -> dict[str, Any]:
    from taskmaestro.discovery import get_registered_workflow, load_registry
    from taskmaestro.exceptions import PluginLoadError

    try:
        workflow = get_registered_workflow(workflow_id)
    except PluginLoadError as exc:
        raise DefinitionError(str(exc)) from exc
    tasks = load_registry(include_workflows=False).tasks
    return definition_from_workflow(
        workflow, workflow_id, class_ids(tasks), describe=describe
    )


def catalog_main(argv: list[str]) -> dict[str, Any]:
    import argparse

    parser = argparse.ArgumentParser(
        prog="catalog", description="List installed taskmaestro tasks and workflows."
    )
    parser.add_argument("--no-workflows", action="store_true")
    args = parser.parse_args(argv)
    return build_catalog(include_workflows=not args.no_workflows)


def export_registered_main(argv: list[str]) -> dict[str, Any]:
    import argparse

    parser = argparse.ArgumentParser(
        prog="export-registered",
        description="Convert a registered workflow to a workflow definition.",
    )
    parser.add_argument("workflow_id")
    parser.add_argument("--describe", action="store_true")
    args = parser.parse_args(argv)
    result = export_registered(args.workflow_id, describe=args.describe)
    result["status"] = "ok"
    return result
