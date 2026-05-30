"""Execute a taskmaestro workflow against a running ResInsight.

The helper:
1. Connects back to ResInsight via the gRPC port passed by the caller.
2. Reads input.yaml and walks the dict, replacing every
   `{__resinsight_ref__: <type>, ...id...}` map with an `ObjectModel`-style
   `{"value": <live rips object>}` so taskmaestro's Pydantic validation
   accepts it as a wrapped object.
3. Builds the workflow via taskmaestro's loader and runs it.
4. Streams progress events as newline-delimited JSON to stdout for the
   ResInsight log dialog to render.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any, Callable

import yaml

from .refs import REF_MARKER


def _emit(event: str, **fields: Any) -> None:
    sys.stdout.write(json.dumps({"event": event, **fields}) + "\n")
    sys.stdout.flush()


def resolve_refs(data: Any, resolver: Callable[[str, dict[str, Any]], Any]) -> Any:
    """Recursively replace `{__resinsight_ref__: ..., ...}` maps in `data`.

    The replacement is a `{"value": <resolved>}` dict, so Pydantic's
    ObjectModel validator can accept it directly.
    """
    if isinstance(data, dict):
        if REF_MARKER in data:
            ri_type = data[REF_MARKER]
            extras = {k: v for k, v in data.items() if k != REF_MARKER}
            return {"value": resolver(ri_type, extras)}
        return {k: resolve_refs(v, resolver) for k, v in data.items()}
    if isinstance(data, list):
        return [resolve_refs(v, resolver) for v in data]
    return data


def make_rips_resolver(rips_instance: Any) -> Callable[[str, dict[str, Any]], Any]:
    """Resolve a `__resinsight_ref__` map to a live `rips` object."""

    def resolve(ri_type: str, extras: dict[str, Any]) -> Any:
        project = rips_instance.project
        if ri_type == "EclipseCase":
            return project.case(case_id=extras["case_id"])
        if ri_type == "WellPath":
            return project.well_path_by_name(extras["well_path_name"])
        if ri_type == "View":
            for v in project.views():
                if getattr(v, "id", None) == extras["view_id"]:
                    return v
            raise LookupError(f"View id {extras['view_id']} not found")
        raise LookupError(f"Unknown resinsight_type: {ri_type}")

    return resolve


def run_workflow(workflow_dir: Path, input_path: Path, grpc_port: int) -> int:
    workflow_dir_str = str(workflow_dir)
    if workflow_dir_str not in sys.path:
        sys.path.insert(0, workflow_dir_str)

    import rips

    instance = rips.Instance.find(start_port=grpc_port, end_port=grpc_port + 1)
    if instance is None:
        _emit("error", message=f"Could not connect to ResInsight on port {grpc_port}")
        return 1
    _emit("connected", port=grpc_port)

    raw_input = yaml.safe_load(input_path.read_text()) or {}
    if not isinstance(raw_input, dict):
        _emit("error", message="input.yaml must contain a mapping")
        return 1

    resolver = make_rips_resolver(instance)
    try:
        resolved = resolve_refs(raw_input, resolver)
    except Exception as exc:
        _emit("error", message=f"Failed to resolve references: {exc}")
        return 1

    from taskmaestro import EmptyConfig, ExecutionContext, Job, JobConfiguration, Runner
    from taskmaestro.yaml_config import _load_workflow_only

    workflow_yaml = workflow_dir / "workflow.yaml"
    workflow, _ = _load_workflow_only(workflow_yaml)

    job_config = JobConfiguration(resolved)
    job: Job[Any] = Job(
        workflow=workflow, config=EmptyConfig(), job_configuration=job_config
    )

    _emit("starting", workflow=workflow.name)
    try:
        result = Runner().run(job, ctx=ExecutionContext())
    except Exception as exc:
        _emit("error", message=str(exc))
        return 1

    _emit("finished", status=str(result.status), error=str(result.error or ""))
    return 0 if result.error is None else 1


def main(argv: list[str]) -> int:
    import argparse

    parser = argparse.ArgumentParser(
        prog="run", description="Run a taskmaestro workflow."
    )
    parser.add_argument("workflow_dir", type=Path)
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--grpc-port", type=int, required=True)
    args = parser.parse_args(argv)
    return run_workflow(
        args.workflow_dir.resolve(), args.input.resolve(), args.grpc_port
    )
