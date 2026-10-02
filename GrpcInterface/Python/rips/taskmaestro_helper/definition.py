"""Workflow definitions exchanged between ResInsight and taskmaestro YAML files.

A *definition* is a JSON document that describes a workflow as a plain graph:
task instances (``nodes``), connections between task outputs and inputs
(``edges``) and per-task configuration values (``inputs``). ResInsight edits
definitions; this module converts them from and to the standard
``workflow.yaml``/``input.yaml`` pair and from registered workflows.
"""

from __future__ import annotations

import os
import shutil
import sys
import tempfile
from collections.abc import Iterator
from contextlib import contextmanager
from pathlib import Path
from typing import Any

import yaml

from ._compat import HelperError
from .describe import describe_workflow, describe_yaml
from .refs import REF_MARKER
from .schema import task_descriptor

DEFINITION_FORMAT = 1
WORKFLOW_FILE = "workflow.yaml"
INPUT_FILE = "input.yaml"
BACKUP_SUFFIX = ".bak"


# --- Small helpers ---


def _edge(
    upstream: str, output: str | None, downstream: str, input_field: str | None
) -> dict[str, Any]:
    return {"from": upstream, "output": output, "to": downstream, "input": input_field}


def _empty_definition(name: str) -> dict[str, Any]:
    return {
        "format": DEFINITION_FORMAT,
        "name": name,
        "result_task": None,
        "editable": True,
        "readonly_reasons": [],
        "warnings": [],
        "source": {"workflow_yaml": None, "input_yaml": None, "registered_id": None},
        "header_comment": "",
        "passthrough": {},
        "nodes": [],
        "edges": [],
        "inputs": {},
        "task_types": {},
        "describe": None,
        "describe_error": None,
    }


def header_comment(text: str) -> str:
    """Return the comment block at the top of a YAML file."""
    lines: list[str] = []
    for line in text.splitlines():
        stripped = line.strip()
        if stripped.startswith("#"):
            lines.append(line.rstrip())
        elif stripped == "" and lines:
            lines.append("")
        elif stripped != "":
            break
    while lines and lines[-1] == "":
        lines.pop()
    return "\n".join(lines)


@contextmanager
def workflow_imports(directory: Path) -> Iterator[None]:
    """Make task modules next to a workflow file importable while loading."""
    original_path = sys.path.copy()
    sys.path.insert(0, str(directory.resolve()))
    try:
        yield
    finally:
        sys.path[:] = original_path


def _jsonable(value: Any, path: str) -> Any:
    from taskmaestro.cli import _jsonable as taskmaestro_jsonable

    try:
        return taskmaestro_jsonable(value, path)
    except (TypeError, ValueError) as exc:
        raise HelperError(str(exc)) from exc


def _yaml_load(text: str, what: str) -> Any:
    from taskmaestro.yaml_config import _yaml_load as taskmaestro_yaml_load

    try:
        return taskmaestro_yaml_load(text)
    except yaml.YAMLError as exc:
        raise HelperError(f"{what} parse error: {exc}") from exc


def strip_refs(value: Any) -> Any:
    """Remove ResInsight object references, which are only valid for one run."""
    if isinstance(value, dict):
        return {
            key: strip_refs(item)
            for key, item in value.items()
            if not (isinstance(item, dict) and REF_MARKER in item)
        }
    if isinstance(value, list):
        return [
            strip_refs(item)
            for item in value
            if not (isinstance(item, dict) and REF_MARKER in item)
        ]
    return value


def _is_identifier(name: str) -> bool:
    return isinstance(name, str) and name.isidentifier()


# --- Loading YAML ---


class _TaskResolver:
    """Resolve the ``task:`` strings of a YAML file like taskmaestro's loader."""

    def __init__(self) -> None:
        from taskmaestro.discovery import registered_task_names

        self.installed = registered_task_names()

    def resolve(self, task_id: str) -> type[Any]:
        from taskmaestro.discovery import get_registered_task
        from taskmaestro.exceptions import ConfigLoadError, PluginLoadError
        from taskmaestro.task import Task
        from taskmaestro.yaml_config import import_class

        try:
            if task_id in self.installed:
                cls = get_registered_task(task_id)
            else:
                cls = import_class(task_id)
        except (ConfigLoadError, PluginLoadError) as exc:
            raise HelperError(str(exc)) from exc
        if not (isinstance(cls, type) and issubclass(cls, Task)):
            raise HelperError(f"'{task_id}' is not a Task subclass")
        return cls


def _inner_workflow_name(base_dir: Path, workflow_path: str) -> str:
    try:
        raw = yaml.safe_load((base_dir / workflow_path).read_text())
        return str(raw["workflow"]["name"])
    except Exception:
        return Path(workflow_path).stem


def _add_task_type(definition: dict[str, Any], task_id: str, cls: type[Any]) -> None:
    if task_id in definition["task_types"]:
        return
    try:
        definition["task_types"][task_id] = task_descriptor(task_id, cls)
    except Exception as exc:
        definition["readonly_reasons"].append(
            f"Cannot describe task '{task_id}': {exc}"
        )


def _load_nodes(
    definition: dict[str, Any], config: Any, base_dir: Path
) -> dict[str, set[str]]:
    """Create one node per YAML entry and return the reference lookup."""
    resolver = _TaskResolver()
    candidates: dict[str, set[str]] = {}
    for entry in config.workflow.tasks:
        node: dict[str, Any] = {"name": "", "task": entry.task, "config_fields": []}
        if entry.workflow:
            key = entry.workflow
            name = entry.name or _inner_workflow_name(base_dir, entry.workflow)
            node["workflow"] = entry.workflow
            definition["readonly_reasons"].append(
                f"Task '{name}' is a nested workflow ('{entry.workflow}')"
            )
        else:
            key = entry.task
            try:
                cls = resolver.resolve(entry.task)
            except HelperError as exc:
                cls = None
                definition["readonly_reasons"].append(
                    f"Cannot load task '{entry.task}': {exc.message}"
                )
            name = entry.name or (cls.name if cls is not None else entry.task)
            if cls is not None:
                _add_task_type(definition, entry.task, cls)
        if entry.map is not None:
            node["map"] = entry.map.model_dump()
            definition["readonly_reasons"].append(f"Task '{name}' is mapped ('map:')")
        if any(existing["name"] == name for existing in definition["nodes"]):
            raise HelperError(f"Duplicate task name '{name}'", task=name)
        node["name"] = name
        node["config_fields"] = list(entry.config_fields or [])
        node["_declared_config"] = entry.config_fields is not None
        definition["nodes"].append(node)
        candidates.setdefault(key, set()).add(name)
        if entry.name:
            candidates.setdefault(entry.name, set()).add(name)
    return candidates


def _resolve_reference(candidates: dict[str, set[str]], ref: str, context: str) -> str:
    names = candidates.get(ref)
    where = f" for task '{context}'" if context else ""
    if names is None:
        raise HelperError(f"Dependency '{ref}'{where} not found", task=context or None)
    if len(names) > 1:
        raise HelperError(
            f"Dependency '{ref}'{where} is ambiguous; it matches {sorted(names)}. "
            "Use the instance name.",
            task=context or None,
        )
    return next(iter(names))


def _field_reference(raw: Any, task: str, field: str | None) -> tuple[str, str | None]:
    if isinstance(raw, str):
        return raw, None
    if (
        isinstance(raw, list)
        and len(raw) == 2
        and all(isinstance(item, str) for item in raw)
    ):
        return raw[0], raw[1]
    where = f" for field '{field}'" if field else ""
    raise HelperError(
        f"Invalid dependency {raw!r}{where} on task '{task}'", task=task, field=field
    )


def _load_edges(
    definition: dict[str, Any], config: Any, candidates: dict[str, set[str]]
) -> None:
    entries = config.workflow.tasks
    nodes = definition["nodes"]
    linear = all(entry.depends_on is None and entry.map is None for entry in entries)
    if linear:
        for previous, node in zip(nodes, nodes[1:]):
            definition["edges"].append(
                _edge(previous["name"], None, node["name"], None)
            )
        if len(nodes) > 1:
            definition["warnings"].append(
                "The workflow has no 'depends_on' entries; tasks were chained in "
                "file order and the dependencies will be saved explicitly"
            )
        return

    for entry, node in zip(entries, nodes):
        name = node["name"]
        deps = entry.depends_on
        if deps is None:
            continue
        if isinstance(deps, dict):
            for field, raw in deps.items():
                if isinstance(raw, dict) and set(raw) == {"collect"}:
                    definition["readonly_reasons"].append(
                        f"Task '{name}' collects outputs into '{field}' ('collect:')"
                    )
                    continue
                ref, output = _field_reference(raw, name, field)
                upstream = _resolve_reference(candidates, ref, name)
                definition["edges"].append(_edge(upstream, output, name, field))
            continue
        ref, output = _field_reference(deps, name, None)
        upstream = _resolve_reference(candidates, ref, name)
        definition["edges"].append(_edge(upstream, output, name, None))


def _load_inputs(definition: dict[str, Any], input_path: Path | None) -> None:
    raw_input: Any = {}
    if input_path is not None and input_path.exists():
        raw_input = _yaml_load(input_path.read_text(), "Input YAML") or {}
    if not isinstance(raw_input, dict):
        raise HelperError("Input YAML file must contain a mapping")

    nodes = {node["name"]: node for node in definition["nodes"]}
    for key, values in raw_input.items():
        if key not in nodes:
            definition["warnings"].append(
                f"Dropped input values for unknown task '{key}'"
            )
            continue
        if values is None:
            values = {}
        if not isinstance(values, dict):
            raise HelperError(
                f"Input value for task '{key}' must be a mapping", task=key
            )
        definition["inputs"][key] = _jsonable(values, str(key))

    for node in nodes.values():
        values = definition["inputs"].setdefault(node["name"], {})
        map_source = node.get("map", {}).get("over") if node.get("map") else None
        if not node.pop("_declared_config"):
            # taskmaestro uses the input keys when config_fields is omitted.
            node["config_fields"] = [field for field in values if field != map_source]
        wired = {
            edge["input"]
            for edge in definition["edges"]
            if edge["to"] == node["name"] and edge["input"] is not None
        }
        for field in sorted(wired & set(node["config_fields"])):
            node["config_fields"].remove(field)
            if field in values:
                definition["warnings"].append(
                    f"Input '{node['name']}.{field}' is connected to an upstream task; "
                    "its configured value was dropped"
                )
                del values[field]
        if definition["editable"]:
            for field in [f for f in values if f not in node["config_fields"]]:
                if field == map_source:
                    continue
                definition["warnings"].append(
                    f"Dropped value for '{node['name']}.{field}', which is not a "
                    "configuration field"
                )
                del values[field]
    definition["inputs"] = {name: definition["inputs"][name] for name in nodes}


def load_definition(workflow_dir: Path, *, describe: bool = False) -> dict[str, Any]:
    """Load ``workflow.yaml`` (and ``input.yaml``) from a workflow folder."""
    from pydantic import ValidationError
    from taskmaestro.yaml_config import YamlWorkflowConfig

    workflow_dir = workflow_dir.resolve()
    workflow_path = workflow_dir / WORKFLOW_FILE
    input_path = workflow_dir / INPUT_FILE
    try:
        text = workflow_path.read_text()
    except OSError as exc:
        raise HelperError(f"Cannot read '{workflow_path}': {exc}") from exc
    raw = _yaml_load(text, "YAML")
    if not isinstance(raw, dict):
        raise HelperError("YAML file must contain a mapping at top level")
    try:
        config = YamlWorkflowConfig.model_validate(raw)
    except ValidationError as exc:
        raise HelperError(f"YAML schema validation error: {exc}") from exc

    definition = _empty_definition(config.workflow.name)
    definition["source"] = {
        "workflow_yaml": str(workflow_path),
        "input_yaml": str(input_path) if input_path.exists() else None,
        "registered_id": None,
    }
    definition["header_comment"] = header_comment(text)
    definition["passthrough"] = {
        key: _jsonable(value, str(key))
        for key, value in raw.items()
        if key != "workflow"
    }

    with workflow_imports(workflow_dir):
        candidates = _load_nodes(definition, config, workflow_dir)
        _load_edges(definition, config, candidates)
        if config.workflow.result_task:
            definition["result_task"] = _resolve_reference(
                candidates, config.workflow.result_task, ""
            )
        definition["editable"] = not definition["readonly_reasons"]
        _load_inputs(definition, input_path)

        if describe:
            definition["describe"], definition["describe_error"] = describe_yaml(
                workflow_path, input_path if input_path.exists() else None
            )
    return definition


# --- Registered workflows ---


def _class_task_id(cls: type[Any], class_ids: dict[type[Any], str]) -> str | None:
    """Return a task identifier that resolves back to ``cls``, if any."""
    from taskmaestro.exceptions import ConfigLoadError
    from taskmaestro.yaml_config import import_class

    if cls in class_ids:
        return class_ids[cls]
    if "<locals>" in cls.__qualname__:
        return None
    path = f"{cls.__module__}.{cls.__qualname__}"
    try:
        return path if import_class(path) is cls else None
    except ConfigLoadError:
        return None


def definition_from_workflow(
    workflow: Any,
    registered_id: str | None,
    class_ids: dict[type[Any], str],
    *,
    describe: bool = False,
) -> dict[str, Any]:
    """Convert an in-memory workflow to a definition.

    ``class_ids`` maps registered task classes to their entry-point names.
    """
    from taskmaestro.dependencies import CollectionRef

    definition = _empty_definition(workflow.name)
    definition["source"]["registered_id"] = registered_id
    reasons = definition["readonly_reasons"]

    has_dependents: set[str] = set()
    for name, cls in workflow.topological_order():
        node: dict[str, Any] = {
            "name": name,
            "task": None,
            "config_fields": sorted(workflow.get_config_fields(name)),
        }
        if hasattr(cls, "_inner_workflow"):
            reasons.append(f"Task '{name}' is a nested workflow")
        else:
            task_id = _class_task_id(cls, class_ids)
            if task_id is None:
                reasons.append(
                    f"Task '{name}' uses class '{cls.__module__}.{cls.__qualname__}', "
                    "which cannot be referenced from YAML"
                )
            else:
                node["task"] = task_id
                _add_task_type(definition, task_id, cls)
        if workflow.get_task_map(name) is not None:
            reasons.append(f"Task '{name}' is mapped")
        definition["nodes"].append(node)
        definition["inputs"][name] = {}

        deps = workflow.get_dependencies(name)
        if deps is None:
            continue
        if isinstance(deps, str):
            definition["edges"].append(_edge(deps, None, name, None))
        elif isinstance(deps, tuple):
            definition["edges"].append(_edge(deps[0], deps[1], name, None))
        else:
            for field, ref in deps.items():
                if isinstance(ref, CollectionRef):
                    reasons.append(f"Task '{name}' collects outputs into '{field}'")
                    has_dependents.update(item.task_name for item in ref.output_refs())
                elif isinstance(ref, tuple):
                    definition["edges"].append(_edge(ref[0], ref[1], name, field))
                else:
                    definition["edges"].append(_edge(ref, None, name, field))

    has_dependents.update(edge["from"] for edge in definition["edges"])
    sinks = [
        node["name"]
        for node in definition["nodes"]
        if node["name"] not in has_dependents
    ]
    if sinks != [workflow.result_task_name]:
        definition["result_task"] = workflow.result_task_name
    definition["editable"] = not reasons
    if describe:
        try:
            definition["describe"] = describe_workflow(workflow)
        except Exception as exc:
            definition["describe_error"] = {
                "code": "describe_error",
                "message": str(exc),
            }
    return definition


# --- Saving YAML ---


class _FlowList(list[Any]):
    """A list written in YAML flow style (``[a, b]``)."""


class _Dumper(yaml.SafeDumper):
    def increase_indent(self, flow: bool = False, indentless: bool = False) -> Any:
        return super().increase_indent(flow, False)


def _represent_flow_list(dumper: yaml.SafeDumper, data: _FlowList) -> Any:
    return dumper.represent_sequence("tag:yaml.org,2002:seq", data, flow_style=True)


_Dumper.add_representer(_FlowList, _represent_flow_list)


def _dump_yaml(data: Any) -> str:
    return yaml.dump(
        data,
        Dumper=_Dumper,
        sort_keys=False,
        default_flow_style=False,
        allow_unicode=True,
        width=100,
    )


def _reference(upstream: str, output: str | None) -> Any:
    return upstream if output is None else _FlowList([upstream, output])


def _check_saveable(definition: dict[str, Any]) -> None:
    if not definition.get("editable", True):
        reasons = "; ".join(definition.get("readonly_reasons", []))
        raise HelperError(f"The workflow is read-only and cannot be saved: {reasons}")
    if not _is_identifier(definition.get("name", "")):
        raise HelperError(f"Invalid workflow name '{definition.get('name', '')}'")
    nodes = definition.get("nodes", [])
    if not nodes:
        raise HelperError("The workflow has no tasks")
    names: set[str] = set()
    for node in nodes:
        name = node.get("name")
        if not _is_identifier(name):
            raise HelperError(f"Invalid task name '{name}'", task=name)
        if name in names:
            raise HelperError(f"Duplicate task name '{name}'", task=name)
        if not node.get("task"):
            raise HelperError(f"Task '{name}' has no task type", task=name)
        names.add(name)
    for edge in definition.get("edges", []):
        for end in ("from", "to"):
            if edge.get(end) not in names:
                raise HelperError(
                    f"Connection refers to unknown task '{edge.get(end)}'"
                )
    result_task = definition.get("result_task")
    if result_task is not None and result_task not in names:
        raise HelperError(f"Unknown result task '{result_task}'")


def _dependencies(definition: dict[str, Any], node_name: str) -> Any:
    incoming = [edge for edge in definition.get("edges", []) if edge["to"] == node_name]
    whole = [edge for edge in incoming if edge.get("input") is None]
    fields = [edge for edge in incoming if edge.get("input") is not None]
    if whole and fields:
        raise HelperError(
            f"Task '{node_name}' cannot take its whole input from one task and "
            "individual input fields from others",
            task=node_name,
        )
    if len(whole) > 1:
        raise HelperError(
            f"Task '{node_name}' has more than one connection to its whole input",
            task=node_name,
        )
    if whole:
        return _reference(whole[0]["from"], whole[0].get("output"))
    if not fields:
        return None
    deps: dict[str, Any] = {}
    for edge in fields:
        if edge["input"] in deps:
            raise HelperError(
                f"Input '{node_name}.{edge['input']}' has more than one connection",
                task=node_name,
                field=edge["input"],
            )
        deps[edge["input"]] = _reference(edge["from"], edge.get("output"))
    return deps


def workflow_document(definition: dict[str, Any]) -> dict[str, Any]:
    """Build the ``workflow.yaml`` document for a definition."""
    _check_saveable(definition)
    nodes = definition["nodes"]
    has_edges = bool(definition.get("edges"))
    tasks: list[dict[str, Any]] = []
    for index, node in enumerate(nodes):
        entry: dict[str, Any] = {"task": node["task"], "name": node["name"]}
        deps = _dependencies(definition, node["name"])
        if deps is None and index == 0 and len(nodes) > 1 and not has_edges:
            # Without any depends_on taskmaestro chains the tasks in file order.
            deps = {}
        if deps is not None:
            entry["depends_on"] = deps
        if node.get("config_fields"):
            entry["config_fields"] = _FlowList(node["config_fields"])
        tasks.append(entry)

    section: dict[str, Any] = {"name": definition["name"]}
    if definition.get("result_task"):
        section["result_task"] = definition["result_task"]
    section["tasks"] = tasks
    document: dict[str, Any] = {"workflow": section}
    for key, value in (definition.get("passthrough") or {}).items():
        if key != "workflow":
            document[key] = value
    return document


def input_document(definition: dict[str, Any]) -> dict[str, Any]:
    """Build the ``input.yaml`` document: one key per task, literal values only."""
    inputs = definition.get("inputs") or {}
    document: dict[str, Any] = {}
    for node in definition["nodes"]:
        values = inputs.get(node["name"]) or {}
        config_fields = node.get("config_fields") or []
        document[node["name"]] = strip_refs(
            {field: value for field, value in values.items() if field in config_fields}
        )
    return document


def _write_atomically(path: Path, text: str) -> None:
    fd, temp_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as handle:
            handle.write(text)
        os.replace(temp_name, path)
    except BaseException:
        Path(temp_name).unlink(missing_ok=True)
        raise


def write_definition(
    definition: dict[str, Any],
    out_dir: Path,
    *,
    workflow_only: bool = False,
    backup: bool = False,
) -> dict[str, Any]:
    """Write ``workflow.yaml`` (and ``input.yaml``) for a definition."""
    workflow_text = "\n".join(
        _dump_yaml({key: value}) for key, value in workflow_document(definition).items()
    )
    comment = (definition.get("header_comment") or "").strip("\n")
    if comment:
        workflow_text = f"{comment}\n\n{workflow_text}"
    input_text = None if workflow_only else _dump_yaml(input_document(definition))

    out_dir.mkdir(parents=True, exist_ok=True)
    workflow_path = out_dir / WORKFLOW_FILE
    input_path = out_dir / INPUT_FILE
    backups: list[str] = []
    if backup:
        for path in (workflow_path, input_path):
            backup_path = path.with_name(path.name + BACKUP_SUFFIX)
            if path.exists() and not backup_path.exists():
                shutil.copy2(path, backup_path)
                backups.append(str(backup_path))

    _write_atomically(workflow_path, workflow_text)
    if input_text is not None:
        _write_atomically(input_path, input_text)
    return {
        "status": "ok",
        "workflow_yaml": str(workflow_path),
        "input_yaml": str(input_path) if input_text is not None else None,
        "backups": backups,
    }


# --- Command-line entry points ---


def load_main(argv: list[str]) -> dict[str, Any]:
    import argparse

    parser = argparse.ArgumentParser(prog="load", description="Load a workflow folder.")
    parser.add_argument("workflow_dir", type=Path)
    parser.add_argument("--describe", action="store_true")
    args = parser.parse_args(argv)
    result = load_definition(args.workflow_dir, describe=args.describe)
    result["status"] = "ok"
    return result


def save_main(argv: list[str], stdin_text: str) -> dict[str, Any]:
    import argparse
    import json

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
        raise HelperError(f"Invalid definition JSON: {exc}") from exc
    if not isinstance(definition, dict):
        raise HelperError("The definition must be a JSON object")

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
