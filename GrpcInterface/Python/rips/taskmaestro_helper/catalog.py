"""Catalog of installed taskmaestro tasks and workflows.

Every entry point is loaded on its own, so one broken plugin is reported in
``errors`` while the rest of the catalog still loads.
"""

from __future__ import annotations

from typing import Any

from ._compat import HelperError, taskmaestro_version
from .definition import definition_from_workflow
from .schema import task_descriptor

CATALOG_FORMAT = 1


def _entry_points(group: str) -> list[Any]:
    # Looked up through the module so tests can monkeypatch discovery.entry_points.
    from taskmaestro import discovery

    return sorted(discovery.entry_points(group=group), key=lambda ep: ep.name)


def _error(
    kind: str, entry_id: str, value: str, exc: Exception | str
) -> dict[str, Any]:
    return {"kind": kind, "id": entry_id, "entry_point": value, "message": str(exc)}


def load_registered_tasks() -> tuple[dict[str, type[Any]], list[dict[str, Any]]]:
    """Load every task entry point. Returns ``(tasks by id, errors)``."""
    from taskmaestro.discovery import TASK_ENTRY_POINT_GROUP
    from taskmaestro.task import Task

    tasks: dict[str, type[Any]] = {}
    errors: list[dict[str, Any]] = []
    for entry_point in _entry_points(TASK_ENTRY_POINT_GROUP):
        if entry_point.name in tasks:
            errors.append(
                _error("task", entry_point.name, entry_point.value, "Duplicate task id")
            )
            continue
        try:
            cls = entry_point.load()
        except Exception as exc:
            errors.append(_error("task", entry_point.name, entry_point.value, exc))
            continue
        if not (isinstance(cls, type) and issubclass(cls, Task)):
            errors.append(
                _error(
                    "task",
                    entry_point.name,
                    entry_point.value,
                    "The entry point is not a Task subclass",
                )
            )
            continue
        tasks[entry_point.name] = cls
    return tasks, errors


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
    from taskmaestro.discovery import WORKFLOW_ENTRY_POINT_GROUP
    from taskmaestro.workflow import Workflow

    tasks, errors = load_registered_tasks()
    descriptors: list[dict[str, Any]] = []
    for task_id, cls in sorted(tasks.items()):
        try:
            descriptors.append(task_descriptor(task_id, cls))
        except Exception as exc:
            errors.append(_error("task", task_id, "", f"Cannot describe task: {exc}"))

    workflows: list[dict[str, Any]] = []
    if include_workflows:
        ids = class_ids(tasks)
        seen: set[str] = set()
        for entry_point in _entry_points(WORKFLOW_ENTRY_POINT_GROUP):
            if entry_point.name in seen:
                errors.append(
                    _error(
                        "workflow",
                        entry_point.name,
                        entry_point.value,
                        "Duplicate workflow id",
                    )
                )
                continue
            seen.add(entry_point.name)
            try:
                workflow = entry_point.load()
                if not isinstance(workflow, Workflow):
                    raise TypeError("The entry point is not a Workflow")
                workflows.append(_workflow_entry(entry_point.name, workflow, ids))
            except Exception as exc:
                errors.append(
                    _error("workflow", entry_point.name, entry_point.value, exc)
                )

    return {
        "format": CATALOG_FORMAT,
        "status": "ok",
        "taskmaestro_version": taskmaestro_version(),
        "tasks": descriptors,
        "workflows": workflows,
        "errors": errors,
    }


def export_registered(workflow_id: str, *, describe: bool = False) -> dict[str, Any]:
    from taskmaestro.discovery import get_registered_workflow
    from taskmaestro.exceptions import PluginLoadError

    try:
        workflow = get_registered_workflow(workflow_id)
    except PluginLoadError as exc:
        raise HelperError(str(exc)) from exc
    tasks, _errors = load_registered_tasks()
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
