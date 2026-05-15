"""Unit tests for rips.taskmaestro_helper.

These tests do not require a running ResInsight; they exercise the
Pydantic introspection logic in isolation against a synthetic workflow
written into a tmp_path so they stay self-contained.
"""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest

pytest.importorskip("taskmaestro")


SYNTHETIC_PIPELINE = '''
"""Synthetic workflow for taskmaestro_helper tests."""

from __future__ import annotations

import datetime
import pathlib

import pydantic
from pydantic import BaseModel, Field

from taskmaestro import EmptyConfig, ExecutionContext, Task


class GreetInput(BaseModel):
    name: str = Field(description="Person to greet", default="world")
    times: int = Field(description="How many times", ge=1)
    when: datetime.date = Field(description="Greet date", default=datetime.date(2024, 1, 1))
    out_file: pathlib.Path = Field(description="Where to write greeting", default=pathlib.Path("/tmp/hi.txt"))
    out_dir: pydantic.DirectoryPath = Field(description="Where to write logs")


class GreetOutput(BaseModel):
    message: str


class Greet(Task[GreetInput, GreetOutput]):
    name = "greet"

    def run(self, input: GreetInput, ctx: ExecutionContext) -> GreetOutput:  # pragma: no cover
        return GreetOutput(message=f"hi {input.name}" * input.times)


class StartInput(BaseModel):
    pass


class StartOutput(BaseModel):
    pass


class Start(Task[StartInput, StartOutput]):
    name = "start"

    def run(self, input: StartInput, ctx: ExecutionContext) -> StartOutput:  # pragma: no cover
        return StartOutput()
'''


SYNTHETIC_WORKFLOW_YAML = """workflow:
  name: synthetic
  tasks:
    - task: pipeline.Start
    - task: pipeline.Greet
      depends_on: pipeline.Start
      config_fields: [name, times, when, out_file, out_dir]
"""


SYNTHETIC_INPUT_YAML = """greet:
  name: alice
  times: 3
  out_dir: /tmp
"""


@pytest.fixture
def workflow_dir(tmp_path: Path) -> Path:
    (tmp_path / "pipeline.py").write_text(SYNTHETIC_PIPELINE)
    (tmp_path / "workflow.yaml").write_text(SYNTHETIC_WORKFLOW_YAML)
    (tmp_path / "input.yaml").write_text(SYNTHETIC_INPUT_YAML)
    return tmp_path


def test_collect_schema_returns_workflow_name_and_tasks(workflow_dir: Path) -> None:
    from rips.taskmaestro_helper.introspect import collect_schema

    schema = collect_schema(workflow_dir)
    assert schema["name"] == "synthetic"
    task_names = [t["name"] for t in schema["tasks"]]
    assert task_names == ["greet"]  # Start has no config_fields, so omitted


def test_collect_schema_extracts_field_types_and_metadata(workflow_dir: Path) -> None:
    from rips.taskmaestro_helper.introspect import collect_schema

    schema = collect_schema(workflow_dir)
    [greet] = schema["tasks"]
    fields_by_name = {f["name"]: f for f in greet["config_fields"]}

    assert fields_by_name["name"]["type"] == "string"
    assert fields_by_name["name"]["default"] == "world"
    assert fields_by_name["name"]["description"] == "Person to greet"
    assert fields_by_name["name"]["required"] is False

    assert fields_by_name["times"]["type"] == "integer"
    assert fields_by_name["times"]["required"] is True
    assert "resinsight_type" not in fields_by_name["times"]

    assert fields_by_name["when"]["type"] == "string"
    assert fields_by_name["when"]["format"] == "date"
    assert fields_by_name["when"]["default"] == "2024-01-01"

    assert fields_by_name["out_file"]["type"] == "string"
    assert fields_by_name["out_file"]["format"] == "path"
    assert fields_by_name["out_file"]["default"] == "/tmp/hi.txt"

    assert fields_by_name["out_dir"]["type"] == "string"
    assert fields_by_name["out_dir"]["format"] == "directory-path"
    assert fields_by_name["out_dir"]["required"] is True


def test_resolve_refs_substitutes_object_model_value() -> None:
    from rips.taskmaestro_helper.run import resolve_refs

    sentinel_case = object()
    sentinel_well = object()

    def resolver(ri_type: str, extras: dict) -> object:
        if ri_type == "EclipseCase" and extras == {"case_id": 0}:
            return sentinel_case
        if ri_type == "WellPath" and extras == {"well_path_name": "B-2H"}:
            return sentinel_well
        raise AssertionError(f"unexpected ref: {ri_type} {extras}")

    raw = {
        "load_model": {
            "case": {"__resinsight_ref__": "EclipseCase", "case_id": 0},
            "label": "x",
        },
        "load_well": {
            "well": {"__resinsight_ref__": "WellPath", "well_path_name": "B-2H"},
        },
        "untouched": "stays",
    }
    out = resolve_refs(raw, resolver)
    assert out["load_model"]["case"] == {"value": sentinel_case}
    assert out["load_model"]["label"] == "x"
    assert out["load_well"]["well"] == {"value": sentinel_well}
    assert out["untouched"] == "stays"


def test_main_emits_json_to_stdout(workflow_dir: Path) -> None:
    rips_root = Path(__file__).resolve().parents[2]
    proc = subprocess.run(
        [
            sys.executable,
            "-m",
            "rips.taskmaestro_helper",
            "introspect",
            str(workflow_dir),
        ],
        cwd=rips_root,
        capture_output=True,
        text=True,
        check=True,
    )
    payload = json.loads(proc.stdout)
    assert payload["name"] == "synthetic"
    assert {t["name"] for t in payload["tasks"]} == {"greet"}
