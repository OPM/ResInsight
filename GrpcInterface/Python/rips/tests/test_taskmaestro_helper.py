"""Unit tests for rips.taskmaestro_helper.

These tests do not require a running ResInsight; they exercise reference
resolution and progress reporting for the `run` subcommand in isolation.
"""

from __future__ import annotations

import json
from pathlib import Path

import pytest

pytest.importorskip("taskmaestro")


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


def test_progress_hook_reports_named_task_instances(
    capsys: pytest.CaptureFixture[str],
) -> None:
    from types import SimpleNamespace

    from rips.taskmaestro_helper.run import PROGRESS_PREFIX, ProgressHook

    hook = ProgressHook("run-123")
    task = SimpleNamespace(name="add_perf_1")
    hook.on_task_start(None, task)
    hook.on_task_complete(None, task, None)
    hook.on_task_fail(None, task, ValueError("bad range"))

    lines = capsys.readouterr().out.splitlines()
    assert len(lines) == 3
    events = [json.loads(line.removeprefix(PROGRESS_PREFIX)) for line in lines]
    assert all(line.startswith(PROGRESS_PREFIX) for line in lines)
    assert [event["state"] for event in events] == ["running", "completed", "failed"]
    assert all(
        event["task"] == "add_perf_1" and event["run_id"] == "run-123"
        for event in events
    )
    assert events[-1]["error"] == "bad range"
    assert all("output" not in event for event in events)


def test_progress_hook_follows_runner_instance_names(
    capsys: pytest.CaptureFixture[str],
) -> None:
    from taskmaestro import EmptyConfig, ExecutionContext, Job, Runner, Task, Workflow

    from rips.taskmaestro_helper.run import PROGRESS_PREFIX, ProgressHook

    class Step(Task[EmptyConfig, EmptyConfig]):
        name = "step"

        def run(self, input: EmptyConfig, ctx: ExecutionContext) -> EmptyConfig:
            return EmptyConfig()

    workflow = (
        Workflow.builder("two_steps")
        .add_task(Step, name="first")
        .add_task(Step, name="second", depends_on="first")
        .build()
    )
    Runner(hooks=[ProgressHook("run-456")]).run(
        Job(workflow, EmptyConfig()), ctx=ExecutionContext()
    )
    events = [
        json.loads(line.removeprefix(PROGRESS_PREFIX))
        for line in capsys.readouterr().out.splitlines()
        if line.startswith(PROGRESS_PREFIX)
    ]
    assert [(event["task"], event["state"]) for event in events] == [
        ("first", "running"),
        ("first", "completed"),
        ("second", "running"),
        ("second", "completed"),
    ]


def test_progress_hook_reports_runner_failure(
    capsys: pytest.CaptureFixture[str],
) -> None:
    from taskmaestro import EmptyConfig, ExecutionContext, Job, Runner, Task, Workflow

    from rips.taskmaestro_helper.run import PROGRESS_PREFIX, ProgressHook

    class Fails(Task[EmptyConfig, EmptyConfig]):
        name = "fails"

        def run(self, input: EmptyConfig, ctx: ExecutionContext) -> EmptyConfig:
            raise ValueError("bad range")

    workflow = Workflow.builder("failure").add_task(Fails, name="add_perf_1").build()
    result = Runner(hooks=[ProgressHook("run-789")]).run(
        Job(workflow, EmptyConfig()), ctx=ExecutionContext()
    )
    assert result.error == "bad range"
    events = [
        json.loads(line.removeprefix(PROGRESS_PREFIX))
        for line in capsys.readouterr().out.splitlines()
        if line.startswith(PROGRESS_PREFIX)
    ]
    assert [(event["task"], event["state"]) for event in events] == [
        ("add_perf_1", "running"),
        ("add_perf_1", "failed"),
    ]
    assert events[-1]["error"] == "bad range"


def test_run_helper_emits_progress_without_grpc(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    import rips

    from rips.taskmaestro_helper.run import PROGRESS_PREFIX, run_workflow

    (tmp_path / "progress_pipeline.py").write_text(
        "from taskmaestro import EmptyConfig, ExecutionContext, Task\n"
        "class Start(Task[EmptyConfig, EmptyConfig]):\n"
        "    name = 'start'\n"
        "    def run(self, input: EmptyConfig, ctx: ExecutionContext) -> EmptyConfig:\n"
        "        return EmptyConfig()\n"
        "class Done(Task[EmptyConfig, EmptyConfig]):\n"
        "    name = 'done'\n"
        "    def run(self, input: EmptyConfig, ctx: ExecutionContext) -> EmptyConfig:\n"
        "        return EmptyConfig()\n"
    )
    (tmp_path / "workflow.yaml").write_text(
        "workflow:\n  name: progress_test\n  tasks:\n"
        "    - task: progress_pipeline.Start\n    - task: progress_pipeline.Done\n"
    )
    input_path = tmp_path / "input.yaml"
    input_path.write_text("{}\n")
    monkeypatch.setattr(rips.Instance, "find", lambda **kwargs: object())

    assert run_workflow(tmp_path, input_path, 1234, "run-456") == 0
    events = [
        json.loads(line.removeprefix(PROGRESS_PREFIX))
        for line in capsys.readouterr().out.splitlines()
        if line.startswith(PROGRESS_PREFIX)
    ]
    assert [(event["task"], event["state"]) for event in events] == [
        ("start", "running"),
        ("start", "completed"),
        ("done", "running"),
        ("done", "completed"),
    ]
    assert all(event["run_id"] == "run-456" for event in events)
