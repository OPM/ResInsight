"""Tests for rips.mcp_server, the MCP server for ResInsight.

Each test calls the tools through an in-process MCP client.
"""

from __future__ import annotations

import os
import sys
from typing import Any

import pytest

pytest.importorskip("mcp")

import anyio  # noqa: E402
from mcp import Client, StdioServerParameters  # noqa: E402

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import dataroot  # noqa: E402
from rips.mcp_server.server import create_server, statistics  # noqa: E402

GRID_CASE = "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
SUMMARY_CASE = "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"

TOOLS = {
    "instance_info",
    "list_cases",
    "case_info",
    "list_properties",
    "property_statistics",
    "list_wells",
    "list_summary_vectors",
    "summary_values",
    "list_views",
    "list_plots",
    "list_surfaces",
    "snapshot",
}


def call_tool(rips_instance: Any, name: str, **arguments: Any) -> Any:
    async def run() -> Any:
        async with Client(create_server(lambda: rips_instance)) as client:
            return await client.call_tool(name, arguments)

    return anyio.run(run)


def test_statistics() -> None:
    result = statistics([4.0, float("inf"), 1.0, 2.0, 3.0, 5.0])
    assert result == {
        "count": 5,
        "undefined_count": 1,
        "min": 1.0,
        "max": 5.0,
        "mean": 3.0,
        "p10": 1.4,
        "p50": 3.0,
        "p90": 4.6,
    }
    assert statistics([]) == {"count": 0, "undefined_count": 0}


def test_all_tools_are_read_only(rips_instance, initialize_test):
    async def run() -> Any:
        async with Client(create_server(lambda: rips_instance)) as client:
            return await client.list_tools()

    tools = anyio.run(run).tools
    assert {tool.name for tool in tools} == TOOLS
    assert all(tool.annotations.read_only_hint for tool in tools)


def test_stdio_server_lists_tools() -> None:
    # No tool is called, so the server never looks for a ResInsight
    server = StdioServerParameters(
        command=sys.executable, args=["-m", "rips.mcp_server"], env=dict(os.environ)
    )

    async def run() -> Any:
        async with Client(server) as client:
            return await client.list_tools()

    assert {tool.name for tool in anyio.run(run).tools} == TOOLS


def test_instance_info(rips_instance, initialize_test):
    info = call_tool(rips_instance, "instance_info").structured_content
    assert info["resinsight_version"] == rips_instance.version_string()
    assert info["gui"] == rips_instance.is_gui()


def test_grid_case_tools(rips_instance, initialize_test):
    case = rips_instance.project.load_case(dataroot.PATH + GRID_CASE)
    view = case.create_view()

    cases = call_tool(rips_instance, "list_cases").structured_content
    assert cases["grid_cases"]["items"] == [
        {"id": case.id, "name": "TEST10K_FLT_LGR_NNC", "type": "EclipseCase"}
    ]

    info = call_tool(rips_instance, "case_info", case_id=case.id).structured_content
    assert info["main_grid_dimensions"] == [90, 96, 36]
    assert info["grid_count"] == 2
    assert info["active_cell_count"] == 11125
    assert info["reservoir_cell_count"] == 316224
    assert info["time_steps"]["total"] == 9
    assert info["time_steps"]["items"][0] == "2014-01-01"

    properties = call_tool(
        rips_instance, "list_properties", case_id=case.id
    ).structured_content
    assert "PORO" in properties["STATIC_NATIVE"]["items"]
    assert "SOIL" in properties["DYNAMIC_NATIVE"]["items"]

    poro = call_tool(
        rips_instance, "property_statistics", case_id=case.id, property="PORO"
    ).structured_content
    assert poro["property_type"] == "STATIC_NATIVE"
    assert poro["count"] == 11125
    assert poro["min"] <= poro["p10"] <= poro["p50"] <= poro["p90"] <= poro["max"]
    assert poro["mean"] == pytest.approx(0.1630879, abs=1e-6)

    soil = call_tool(
        rips_instance,
        "property_statistics",
        case_id=case.id,
        property="SOIL",
        time_step=1,
    ).structured_content
    assert soil["property_type"] == "DYNAMIC_NATIVE"
    assert soil["count"] == 11125

    wells = call_tool(rips_instance, "list_wells", case_id=case.id).structured_content
    assert wells["simulation_wells"]["items"] == ["GI1", "GP1", "GP2"]

    views = call_tool(rips_instance, "list_views").structured_content
    assert views["items"] == [
        {
            "id": view.id,
            "type": "EclipseView",
            "case_id": case.id,
            "current_time_step": 0,
        }
    ]


def test_summary_tools(rips_instance, initialize_test):
    case = rips_instance.project.import_summary_case(dataroot.PATH + SUMMARY_CASE)

    cases = call_tool(rips_instance, "list_cases").structured_content
    assert cases["summary_cases"]["items"] == [
        {"id": case.id, "name": "SIMPLE_SUMMARY2"}
    ]

    vectors = call_tool(
        rips_instance, "list_summary_vectors", case_id=case.id, pattern="FOP?"
    ).structured_content
    assert vectors["items"] == ["FOPR", "FOPT"]

    fopt = call_tool(
        rips_instance, "summary_values", case_id=case.id, vector="FOPT", max_points=4
    ).structured_content
    assert fopt["count"] == 60
    assert fopt["max"] == pytest.approx(51395.199, abs=1e-3)
    assert len(fopt["points"]) == 4
    assert fopt["points"][0]["time"].startswith("2015-04-09")
    assert fopt["points"][-1]["value"] == fopt["max"]


def test_empty_project_lists(rips_instance, initialize_test):
    for name in ("list_views", "list_plots", "list_surfaces"):
        result = call_tool(rips_instance, name).structured_content
        assert result == {"total": 0, "truncated": False, "items": []}


def test_unknown_ids_are_tool_errors(rips_instance, initialize_test):
    case = rips_instance.project.load_case(dataroot.PATH + GRID_CASE)
    for name, arguments, message in [
        ("case_info", {"case_id": 99}, "No grid case with id 99"),
        ("summary_values", {"case_id": 99, "vector": "FOPT"}, "No summary case"),
        (
            "property_statistics",
            {"case_id": case.id, "property": "NOPE"},
            "has no property 'NOPE'",
        ),
        ("snapshot", {"view_id": 99}, "No view or plot with id 99"),
    ]:
        result = call_tool(rips_instance, name, **arguments)
        assert result.is_error
        assert message in result.content[0].text


def test_snapshot(rips_instance, initialize_test):
    if not rips_instance.is_gui():
        pytest.skip("Cannot run test without a GUI")

    case = rips_instance.project.load_case(dataroot.PATH + GRID_CASE)
    view = case.create_view()
    result = call_tool(rips_instance, "snapshot", view_id=view.id)
    # The export is a no-op when the view has no OpenGL viewer (e.g. offscreen Qt platform)
    if not result.is_error:
        assert result.content[0].mime_type == "image/png"
