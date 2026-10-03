"""Tests for the workflow definition subcommands of rips.taskmaestro_helper.

Run with ``pytest --noconftest``: these tests do not need a running ResInsight.
"""

from __future__ import annotations

import json
import shutil
import textwrap
from importlib.metadata import EntryPoint
from pathlib import Path
from typing import Any

import pytest
import yaml

pytest.importorskip("taskmaestro")

from rips.taskmaestro_helper import __main__ as helper_main  # noqa: E402
from rips.taskmaestro_helper.catalog import build_catalog, export_registered  # noqa: E402
from rips.taskmaestro_helper.definition import (  # noqa: E402
    load_definition,
    write_definition,
)
from rips.taskmaestro_helper.describe import describe_yaml  # noqa: E402

TEST_DATA = Path(__file__).parent / "test_data" / "taskmaestro_workflows"
PIPELINE = "tm_helper_pipeline"

STRUCTURAL_KEYS = (
    "name",
    "result_task",
    "nodes",
    "edges",
    "inputs",
    "editable",
    "header_comment",
    "passthrough",
    "task_types",
)


@pytest.fixture(autouse=True)
def pipeline_on_path(monkeypatch: pytest.MonkeyPatch) -> None:
    monkeypatch.syspath_prepend(str(TEST_DATA))


def _fake_entry_points(
    monkeypatch: pytest.MonkeyPatch,
    tasks: dict[str, str],
    workflows: dict[str, str] | None = None,
) -> None:
    from taskmaestro import discovery

    groups = {
        discovery.TASK_ENTRY_POINT_GROUP: [
            EntryPoint(name, value, discovery.TASK_ENTRY_POINT_GROUP)
            for name, value in tasks.items()
        ],
        discovery.WORKFLOW_ENTRY_POINT_GROUP: [
            EntryPoint(name, value, discovery.WORKFLOW_ENTRY_POINT_GROUP)
            for name, value in (workflows or {}).items()
        ],
    }

    def entry_points(*, group: str) -> list[EntryPoint]:
        return groups.get(group, [])

    monkeypatch.setattr(discovery, "entry_points", entry_points)


@pytest.fixture
def plugins(monkeypatch: pytest.MonkeyPatch) -> None:
    _fake_entry_points(
        monkeypatch,
        tasks={
            "test.start": f"{PIPELINE}:Start",
            "test.double": f"{PIPELINE}:Double",
            "test.add": f"{PIPELINE}:Add",
            "test.constant": f"{PIPELINE}:Constant",
            "test.broken": "tm_helper_missing_module:Broken",
        },
        workflows={
            "test.flow": f"{PIPELINE}:registered_workflow",
            "test.local": f"{PIPELINE}:local_workflow",
        },
    )


def _write_workflow(directory: Path, workflow: str, inputs: str | None = None) -> Path:
    directory.mkdir(parents=True, exist_ok=True)
    (directory / "workflow.yaml").write_text(textwrap.dedent(workflow))
    if inputs is not None:
        (directory / "input.yaml").write_text(textwrap.dedent(inputs))
    return directory


def _structure(definition: dict[str, Any]) -> dict[str, Any]:
    return {key: definition[key] for key in STRUCTURAL_KEYS}


def _describe_summary(description: dict[str, Any]) -> dict[str, Any]:
    return {
        "result_task": description["result_task"],
        "tasks": [
            (task["name"], task["depends_on"], task["config_fields"])
            for task in description["tasks"]
        ],
    }


def _round_trip(source: Path, out_dir: Path) -> tuple[dict[str, Any], dict[str, Any]]:
    first = load_definition(source, describe=True)
    assert first["editable"], first["readonly_reasons"]
    assert first["describe_error"] is None, first["describe_error"]
    write_definition(first, out_dir)
    second = load_definition(out_dir, describe=True)
    assert second["describe_error"] is None, second["describe_error"]
    assert _structure(second) == _structure(first)
    assert _describe_summary(second["describe"]) == _describe_summary(first["describe"])
    third_dir = out_dir.parent / (out_dir.name + "_again")
    write_definition(second, third_dir)
    assert (third_dir / "workflow.yaml").read_text() == (
        out_dir / "workflow.yaml"
    ).read_text()
    return first, second


# --- Catalog ---


def test_catalog_reports_broken_plugin_and_loads_the_rest(plugins: None) -> None:
    catalog = build_catalog()
    ids = [task["id"] for task in catalog["tasks"]]
    assert ids == ["test.add", "test.constant", "test.double", "test.start"]
    assert [error["id"] for error in catalog["errors"]] == ["test.broken"]

    start = next(task for task in catalog["tasks"] if task["id"] == "test.start")
    assert start["name"] == "start"
    assert start["description"] == "Produce the configured value."
    assert start["python_type"] == f"{PIPELINE}.Start"
    assert start["input_schema"]["properties"]["value"]["type"] == "number"

    workflows = {workflow["id"]: workflow for workflow in catalog["workflows"]}
    assert set(workflows) == {"test.flow", "test.local"}
    assert workflows["test.flow"]["editable"]
    assert not workflows["test.local"]["editable"]
    assert workflows["test.flow"]["describe"]["result_task"] == "double"


def test_catalog_without_workflows(plugins: None) -> None:
    assert build_catalog(include_workflows=False)["workflows"] == []


def test_catalog_cli_prints_one_json_document(
    plugins: None, capsys: pytest.CaptureFixture[str]
) -> None:
    assert helper_main.main(["catalog", "--no-workflows"]) == 0
    out = capsys.readouterr().out
    document = json.loads(out)
    assert document["status"] == "ok"
    assert len(document["tasks"]) == 4


def test_cli_failure_prints_invalid_document(
    tmp_path: Path, capsys: pytest.CaptureFixture[str]
) -> None:
    assert helper_main.main(["load", str(tmp_path / "missing")]) == 2
    document = json.loads(capsys.readouterr().out)
    assert document["status"] == "invalid"
    assert "Cannot read" in document["error"]["message"]


# --- Schema ---


def test_schema_records_python_bases() -> None:
    from rips.taskmaestro_helper.schema import task_descriptor
    from tm_helper_pipeline import Special

    descriptor = task_descriptor("test.special", Special)
    output = descriptor["output_schema"]
    assert output["x-ri-python-type"] == f"{PIPELINE}.SpecialNumber"
    assert output["x-ri-python-bases"] == [
        f"{PIPELINE}.SpecialNumber",
        f"{PIPELINE}.Number",
    ]


def test_schema_perforation_output_is_not_a_well_path() -> None:
    pytest.importorskip("taskmaestro_resinsight")
    from taskmaestro import get_registered_task

    from rips.taskmaestro_helper.schema import task_descriptor

    add_perforation = task_descriptor(
        "resinsight.add_perforation", get_registered_task("resinsight.add_perforation")
    )
    output_bases = add_perforation["output_schema"]["x-ri-python-bases"]
    well_path = add_perforation["input_schema"]["$defs"]["WellPath"]
    assert well_path["x-ri-python-type"] == "taskmaestro_resinsight.models.WellPath"
    assert well_path["x-ri-python-type"] not in output_bases
    opaque = well_path["properties"]["value"]
    assert opaque["x-taskmaestro-opaque"] is True
    assert "rips.generated.generated_classes.WellPath" in opaque["x-ri-python-bases"]


# --- Round trips ---


def test_round_trip_whole_output_dependency(tmp_path: Path) -> None:
    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: whole
          tasks:
            - task: {PIPELINE}.Start
              config_fields: [value]
            - task: {PIPELINE}.Double
              depends_on: {PIPELINE}.Start
        """,
        "start:\n  value: 2.5\n",
    )
    first, _ = _round_trip(source, tmp_path / "out")
    assert first["edges"] == [
        {"from": "start", "output": None, "to": "double", "input": None}
    ]
    assert first["inputs"] == {"start": {"value": 2.5}, "double": {}}
    text = (tmp_path / "out" / "workflow.yaml").read_text()
    assert "name: start" in text and "depends_on: start" in text
    assert "result_task" not in text


def test_round_trip_output_field_routing(tmp_path: Path) -> None:
    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: routed
          tasks:
            - task: {PIPELINE}.Start
              name: first
              config_fields: [value]
            - task: {PIPELINE}.Wrap
              name: wrap
              depends_on: first
            - task: {PIPELINE}.Double
              name: unwrap
              depends_on: [wrap, inner]
        """,
        "first:\n  value: 1\n",
    )
    first, _ = _round_trip(source, tmp_path / "out")
    assert {"from": "wrap", "output": "inner", "to": "unwrap", "input": None} in first[
        "edges"
    ]
    assert (
        "depends_on: [wrap, inner]" in (tmp_path / "out" / "workflow.yaml").read_text()
    )


def test_round_trip_fan_in_with_field_routing_and_two_instances(tmp_path: Path) -> None:
    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: fan_in
          result_task: add
          tasks:
            - task: {PIPELINE}.Start
              name: left
              config_fields: [value]
            - task: {PIPELINE}.Start
              name: right
              config_fields: [value]
            - task: {PIPELINE}.Add
              name: add
              depends_on:
                first: [left, value]
                second: [right, value]
            - task: {PIPELINE}.Double
              name: side
              depends_on: left
        """,
        "left:\n  value: 1\nright:\n  value: 2\n",
    )
    first, second = _round_trip(source, tmp_path / "out")
    assert first["result_task"] == "add"
    assert [node["name"] for node in second["nodes"]] == [
        "left",
        "right",
        "add",
        "side",
    ]
    assert list(second["task_types"]) == [
        f"{PIPELINE}.Start",
        f"{PIPELINE}.Add",
        f"{PIPELINE}.Double",
    ]
    text = (tmp_path / "out" / "workflow.yaml").read_text()
    assert "result_task: add" in text
    assert "first: [left, value]" in text


def test_load_resolves_class_paths_to_instance_names(tmp_path: Path) -> None:
    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: by_path
          tasks:
            - task: {PIPELINE}.Constant
            - task: {PIPELINE}.Double
              depends_on: {PIPELINE}.Constant
        """,
    )
    definition = load_definition(source)
    assert [node["name"] for node in definition["nodes"]] == ["constant", "double"]
    assert definition["edges"][0]["from"] == "constant"


def test_linear_mode_is_converted_to_explicit_edges(tmp_path: Path) -> None:
    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: linear
          tasks:
            - task: {PIPELINE}.Constant
            - task: {PIPELINE}.Double
            - task: {PIPELINE}.Double
              name: again
        """,
    )
    definition = load_definition(source)
    assert definition["edges"] == [
        {"from": "constant", "output": None, "to": "double", "input": None},
        {"from": "double", "output": None, "to": "again", "input": None},
    ]
    assert any("chained" in warning for warning in definition["warnings"])
    write_definition(definition, tmp_path / "out")
    reloaded = load_definition(tmp_path / "out", describe=True)
    assert reloaded["edges"] == definition["edges"]
    assert reloaded["warnings"] == []
    assert reloaded["describe_error"] is None


def test_empty_depends_on_guard_keeps_unconnected_tasks_independent(
    tmp_path: Path,
) -> None:
    definition = load_definition(
        _write_workflow(
            tmp_path / "src",
            f"""\
            workflow:
              name: independent
              result_task: second
              tasks:
                - task: {PIPELINE}.Constant
                  name: first
                - task: {PIPELINE}.Constant
                  name: second
            """,
        )
    )
    definition["edges"] = []
    write_definition(definition, tmp_path / "out")
    text = (tmp_path / "out" / "workflow.yaml").read_text()
    assert "depends_on: {}" in text
    reloaded = load_definition(tmp_path / "out", describe=True)
    assert reloaded["edges"] == []
    assert reloaded["describe_error"] is None
    assert [task["depends_on"] for task in reloaded["describe"]["tasks"]] == [{}, None]


def test_save_rejects_mixed_whole_and_field_inputs(tmp_path: Path) -> None:
    from rips.taskmaestro_helper._compat import HelperError

    definition = load_definition(
        _write_workflow(
            tmp_path / "src",
            f"""\
            workflow:
              name: mixed
              tasks:
                - task: {PIPELINE}.Constant
                  name: one
                - task: {PIPELINE}.Add
                  name: add
                  depends_on:
                    first: [one, value]
                    second: [one, value]
            """,
        )
    )
    definition["edges"].append(
        {"from": "one", "output": None, "to": "add", "input": None}
    )
    with pytest.raises(HelperError, match="whole input"):
        write_definition(definition, tmp_path / "out")


def test_save_rejects_read_only_and_empty_workflows(tmp_path: Path) -> None:
    from rips.taskmaestro_helper._compat import HelperError

    definition = load_definition(
        _write_workflow(
            tmp_path / "src",
            f"""\
            workflow:
              name: single
              tasks:
                - task: {PIPELINE}.Constant
            """,
        )
    )
    with pytest.raises(HelperError, match="read-only"):
        write_definition({**definition, "editable": False}, tmp_path / "out")
    with pytest.raises(HelperError, match="no tasks"):
        write_definition({**definition, "nodes": []}, tmp_path / "out")


# --- Mapped tasks and collected inputs ---


def test_round_trip_positional_collect(tmp_path: Path) -> None:
    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: collected
          tasks:
            - task: {PIPELINE}.Constant
              name: a
            - task: {PIPELINE}.Constant
              name: b
            - task: {PIPELINE}.Sum
              name: total
              depends_on:
                values:
                  collect: [[b, value], [a, value]]
        """,
    )
    first, _ = _round_trip(source, tmp_path / "out")
    assert [edge for edge in first["edges"] if edge["to"] == "total"] == [
        {
            "from": "b",
            "output": "value",
            "to": "total",
            "input": "values",
            "collect": "list",
            "key": None,
        },
        {
            "from": "a",
            "output": "value",
            "to": "total",
            "input": "values",
            "collect": "list",
            "key": None,
        },
    ]
    raw = yaml.safe_load((tmp_path / "out" / "workflow.yaml").read_text())
    assert raw["workflow"]["tasks"][2]["depends_on"] == {
        "values": {"collect": [["b", "value"], ["a", "value"]]}
    }


def test_round_trip_keyed_collect(tmp_path: Path) -> None:
    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: keyed
          tasks:
            - task: {PIPELINE}.Constant
              name: a
            - task: {PIPELINE}.Constant
              name: b
            - task: {PIPELINE}.KeyedSum
              name: total
              depends_on:
                values:
                  collect: {{first: [a, value], second: [b, value]}}
        """,
    )
    first, _ = _round_trip(source, tmp_path / "out")
    assert [
        (edge["from"], edge["collect"], edge["key"])
        for edge in first["edges"]
        if edge["to"] == "total"
    ] == [("a", "dict", "first"), ("b", "dict", "second")]
    raw = yaml.safe_load((tmp_path / "out" / "workflow.yaml").read_text())
    assert raw["workflow"]["tasks"][2]["depends_on"] == {
        "values": {"collect": {"first": ["a", "value"], "second": ["b", "value"]}}
    }


def test_save_rejects_inconsistent_collect(tmp_path: Path) -> None:
    from rips.taskmaestro_helper._compat import HelperError

    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: keyed
          tasks:
            - task: {PIPELINE}.Constant
              name: a
            - task: {PIPELINE}.Constant
              name: b
            - task: {PIPELINE}.KeyedSum
              name: total
              depends_on:
                values:
                  collect: {{first: [a, value], second: [b, value]}}
        """,
    )
    definition = load_definition(source)
    definition["edges"][-1]["key"] = "first"
    with pytest.raises(HelperError, match="Duplicate key"):
        write_definition(definition, tmp_path / "out")
    definition["edges"][-1]["collect"] = "list"
    with pytest.raises(HelperError, match="mixes"):
        write_definition(definition, tmp_path / "out")


def test_round_trip_mapped_task(tmp_path: Path) -> None:
    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: mapped
          tasks:
            - task: {PIPELINE}.PerItem
              name: items
              map: {{over: numbers, key_as: key, value_as: value, error_mode: collect_all}}
        """,
        "items:\n  numbers:\n    a: 1.0\n    b: 2.0\n",
    )
    first, _ = _round_trip(source, tmp_path / "out")
    assert first["nodes"][0]["map"] == {
        "over": "numbers",
        "key_as": "key",
        "value_as": "value",
        "error_mode": "collect_all",
    }
    assert first["nodes"][0]["config_fields"] == []
    assert first["inputs"] == {"items": {"numbers": {"a": 1.0, "b": 2.0}}}
    raw = yaml.safe_load((tmp_path / "out" / "workflow.yaml").read_text())
    assert raw["workflow"]["tasks"][0]["map"] == {
        "over": "numbers",
        "key_as": "key",
        "value_as": "value",
        "error_mode": "collect_all",
    }
    assert yaml.safe_load((tmp_path / "out" / "input.yaml").read_text()) == {
        "items": {"numbers": {"a": 1.0, "b": 2.0}}
    }


def test_save_rejects_invalid_map(tmp_path: Path) -> None:
    from rips.taskmaestro_helper._compat import HelperError

    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: mapped
          tasks:
            - task: {PIPELINE}.PerItem
              name: items
              map: {{over: numbers, key_as: key, value_as: value}}
        """,
    )
    definition = load_definition(source)
    definition["nodes"][0]["map"]["value_as"] = "key"
    with pytest.raises(HelperError, match="both the key"):
        write_definition(definition, tmp_path / "out")


# --- Read-only workflows ---


@pytest.mark.parametrize(
    ("tasks", "reason"),
    [
        (
            """\
                - workflow: inner.yaml
                  name: nested
            """,
            "nested",
        ),
        (
            """\
                - task: tm_helper_missing_module.Missing
            """,
            "Cannot load task",
        ),
    ],
)
def test_unsupported_features_open_read_only(
    tmp_path: Path, tasks: str, reason: str
) -> None:
    source = tmp_path / "src"
    source.mkdir()
    (source / "inner.yaml").write_text(
        f"workflow:\n  name: inner\n  tasks:\n    - task: {PIPELINE}.Constant\n"
    )
    (source / "workflow.yaml").write_text(
        "workflow:\n  name: read_only\n  tasks:\n"
        + textwrap.indent(textwrap.dedent(tasks), "    ")
    )
    definition = load_definition(source)
    assert not definition["editable"]
    assert any(reason in text for text in definition["readonly_reasons"])


# --- Header, passthrough and references ---


def test_header_and_passthrough_preserved_and_refs_stripped(tmp_path: Path) -> None:
    source = _write_workflow(
        tmp_path / "src",
        f"""\
        # My workflow
        #
        # Explains things.

        workflow:
          name: with_header
          tasks:
            - task: {PIPELINE}.Start
              config_fields: [value]
        # A comment that is lost.
        runner:
          timeout_seconds: 30
          hooks:
            - hook: taskmaestro.hooks.timing.TimingHook
        context:
          correlation_id: abc
        """,
        """\
        start:
          value: {__resinsight_ref__: EclipseCase, case_id: 0}
        """,
    )
    definition = load_definition(source)
    assert definition["header_comment"] == "# My workflow\n#\n# Explains things."
    assert definition["passthrough"] == {
        "runner": {
            "timeout_seconds": 30,
            "hooks": [{"hook": "taskmaestro.hooks.timing.TimingHook"}],
        },
        "context": {"correlation_id": "abc"},
    }
    write_definition(definition, tmp_path / "out", backup=True)
    text = (tmp_path / "out" / "workflow.yaml").read_text()
    assert text.startswith("# My workflow\n#\n# Explains things.\n\nworkflow:")
    assert "A comment that is lost" not in text
    assert "timeout_seconds: 30" in text
    assert "__resinsight_ref__" not in (tmp_path / "out" / "input.yaml").read_text()
    reloaded = load_definition(tmp_path / "out")
    assert reloaded["passthrough"] == definition["passthrough"]
    assert reloaded["inputs"] == {"start": {}}


def test_backup_is_written_once(tmp_path: Path) -> None:
    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: backup
          tasks:
            - task: {PIPELINE}.Constant
        """,
    )
    original = (source / "workflow.yaml").read_text()
    definition = load_definition(source)
    result = write_definition(definition, source, backup=True)
    assert result["backups"] == [str(source / "workflow.yaml.bak")]
    write_definition(definition, source, backup=True)
    assert (source / "workflow.yaml.bak").read_text() == original


def test_wired_input_drops_configured_value(tmp_path: Path) -> None:
    definition = load_definition(
        _write_workflow(
            tmp_path / "src",
            f"""\
            workflow:
              name: wired
              tasks:
                - task: {PIPELINE}.Constant
                  name: one
                - task: {PIPELINE}.Add
                  name: add
                  depends_on:
                    first: [one, value]
                  config_fields: [first, second]
            """,
            "add:\n  first: 1\n  second: 2\nunknown_task:\n  x: 1\n",
        )
    )
    add = next(node for node in definition["nodes"] if node["name"] == "add")
    assert add["config_fields"] == ["second"]
    assert definition["inputs"]["add"] == {"second": 2}
    assert len(definition["warnings"]) == 2


def test_save_cli_reads_definition_from_stdin(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    import io

    definition = load_definition(
        _write_workflow(
            tmp_path / "src",
            f"""\
            workflow:
              name: via_cli
              tasks:
                - task: {PIPELINE}.Start
                  config_fields: [value]
            """,
        )
    )
    monkeypatch.setattr("sys.stdin", io.StringIO(json.dumps(definition)))
    out_dir = tmp_path / "out"
    assert helper_main.main(["save", "--out-dir", str(out_dir), "--describe"]) == 0
    result = json.loads(capsys.readouterr().out)
    assert result["status"] == "ok"
    assert result["describe"]["tasks"][0]["missing_config_fields"] == ["value"]
    assert (out_dir / "input.yaml").read_text() == "start: {}\n"


def test_describe_yaml_reports_real_error_message(tmp_path: Path) -> None:
    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: broken
          tasks:
            - task: {PIPELINE}.Add
              name: add
              depends_on:
                first: {PIPELINE}.Constant
        """,
    )
    description, error = describe_yaml(source / "workflow.yaml")
    assert description is None
    assert error is not None and "not found" in error["message"]


# --- Registered workflows ---


def test_export_registered(plugins: None) -> None:
    definition = export_registered("test.flow", describe=True)
    assert definition["editable"]
    assert definition["source"]["registered_id"] == "test.flow"
    assert definition["nodes"] == [
        {"name": "start", "task": "test.start", "config_fields": ["value"]},
        {"name": "double", "task": "test.double", "config_fields": []},
    ]
    assert definition["edges"] == [
        {"from": "start", "output": None, "to": "double", "input": None}
    ]
    assert definition["result_task"] is None
    assert definition["describe"]["result_task"] == "double"


def test_export_registered_local_class_is_read_only(plugins: None) -> None:
    definition = export_registered("test.local")
    assert not definition["editable"]
    assert "cannot be referenced" in definition["readonly_reasons"][0]


def test_exported_registered_workflow_saves_and_validates(
    plugins: None, tmp_path: Path
) -> None:
    definition = export_registered("test.flow")
    definition["inputs"] = {"start": {"value": 3}}
    write_definition(definition, tmp_path / "out")
    description, error = describe_yaml(
        tmp_path / "out" / "workflow.yaml", tmp_path / "out" / "input.yaml"
    )
    assert error is None
    assert description is not None and description["result_task"] == "double"


def test_run_registered_workflow(
    plugins: None,
    tmp_path: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    import rips

    from rips.taskmaestro_helper.run import PROGRESS_PREFIX, main

    input_path = tmp_path / "input.yaml"
    input_path.write_text("start:\n  value: 2\n")
    monkeypatch.setattr(rips.Instance, "find", lambda **kwargs: object())

    assert (
        main(
            [
                "--registered",
                "test.flow",
                "--input",
                str(input_path),
                "--grpc-port",
                "1234",
                "--run-id",
                "rid",
            ]
        )
        == 0
    )
    events = [
        json.loads(line.removeprefix(PROGRESS_PREFIX))
        for line in capsys.readouterr().out.splitlines()
        if line.startswith(PROGRESS_PREFIX)
    ]
    assert [(event["task"], event["state"]) for event in events] == [
        ("start", "running"),
        ("start", "completed"),
        ("double", "running"),
        ("double", "completed"),
    ]


def test_run_requires_folder_or_registered_id(tmp_path: Path) -> None:
    from rips.taskmaestro_helper.run import main

    with pytest.raises(SystemExit):
        main(["--input", str(tmp_path / "input.yaml"), "--grpc-port", "1"])


# --- The taskmaestro-resinsight examples ---


@pytest.mark.parametrize("name", ["resinsight_completions", "resinsight_load_data"])
def test_example_workflows_round_trip(tmp_path: Path, name: str) -> None:
    pytest.importorskip("taskmaestro_resinsight")
    source = tmp_path / name
    shutil.copytree(TEST_DATA / name, source)
    first, _ = _round_trip(source, tmp_path / "out")
    assert first["warnings"] == []
    assert all(node["task"].startswith("resinsight.") for node in first["nodes"])
