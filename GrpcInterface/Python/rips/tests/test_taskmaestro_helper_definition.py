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

pytest.importorskip("taskmaestro")

from rips.taskmaestro_helper import __main__ as helper_main
from rips.taskmaestro_helper.catalog import build_catalog, export_registered
from rips.taskmaestro_helper.definition import (
    describe_yaml,
    load_definition,
    write_definition,
)

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
    assert ids == ["test.constant", "test.double", "test.start"]
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
    assert len(document["tasks"]) == 3


def test_cli_reports_missing_taskmaestro(
    monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    def missing(argv: list[str]) -> dict[str, Any]:
        raise ModuleNotFoundError("No module named 'taskmaestro'", name="taskmaestro")

    monkeypatch.setitem(helper_main.JSON_COMMANDS, "catalog", missing)
    assert helper_main.main(["catalog"]) == 2
    error = json.loads(capsys.readouterr().out)["error"]
    assert error["message"].startswith("taskmaestro>=0.4,<0.5 is required")


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
    from rips.taskmaestro_helper.schema import task_descriptor
    from taskmaestro import get_registered_task

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


# --- References ---


def test_saved_input_drops_resinsight_refs(tmp_path: Path) -> None:
    source = _write_workflow(
        tmp_path / "src",
        f"""\
        workflow:
          name: with_ref
          tasks:
            - task: {PIPELINE}.Start
              config_fields: [value]
        """,
        "start:\n  value: {__resinsight_ref__: EclipseCase, case_id: 0}\n",
    )
    write_definition(load_definition(source), tmp_path / "out")
    assert (tmp_path / "out" / "input.yaml").read_text() == "start: {}\n"


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
