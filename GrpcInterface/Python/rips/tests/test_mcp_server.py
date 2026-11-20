"""Tests for rips.mcp_server, the MCP server for ResInsight.

Each test calls the tools through an in-process MCP client.
"""

from __future__ import annotations

import os
import sys
import threading
import time
from typing import Any

import pytest

pytest.importorskip("mcp")

import anyio  # noqa: E402
from mcp import Client, StdioServerParameters  # noqa: E402

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import dataroot  # noqa: E402
import rips  # noqa: E402
from rips.mcp_server.__main__ import _Connection  # noqa: E402
from rips.mcp_server.server import create_server  # noqa: E402

GRID_CASE = "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
SUMMARY_CASE = "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"

TOOLS = {
    "instance_info",
    "list_cases",
    "case_info",
    "list_properties",
    "list_wells",
    "list_summary_vectors",
    "summary_values",
    "list_views",
    "list_plots",
    "list_surfaces",
    "snapshot_view",
    "snapshot_plot",
}


def call_tool(rips_instance: Any, name: str, **arguments: Any) -> Any:
    async def run() -> Any:
        async with Client(create_server(lambda: rips_instance)) as client:
            return await client.call_tool(name, arguments)

    return anyio.run(run)


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
    assert info["time_steps"]["items"][0] == "2014-01-01T00:00:00+00:00"

    properties = call_tool(
        rips_instance, "list_properties", case_id=case.id
    ).structured_content
    assert "PORO" in properties["STATIC_NATIVE"]["items"]
    assert "SOIL" in properties["DYNAMIC_NATIVE"]["items"]

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


def test_views_with_an_intersection(rips_instance, initialize_test):
    # An intersection adds a 2D intersection view, which has no case
    case_root = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case = rips_instance.project.load_case(case_root + "/TEST10K_FLT_LGR_NNC.EGRID")
    view = case.create_view()
    rips_instance.project.import_well_paths([case_root + "/wellpath_a.dev"])
    intersection_coll = rips_instance.project.descendants(rips.IntersectionCollection)[
        0
    ]
    intersection = intersection_coll.add_new_object(rips.CurveIntersection)
    intersection.type = "CS_WELL_PATH"
    intersection.well_path = rips_instance.project.well_paths()[0]
    intersection.update()
    assert len(rips_instance.project.views()) == 2

    views = call_tool(rips_instance, "list_views")
    assert not views.is_error
    assert [item["id"] for item in views.structured_content["items"]] == [view.id]

    wells = call_tool(rips_instance, "list_wells", case_id=case.id).structured_content
    assert wells["view_count"] == 1
    assert wells["simulation_wells"]["items"] == ["GI1", "GP1", "GP2"]


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

    lower_case = call_tool(
        rips_instance, "list_summary_vectors", case_id=case.id, pattern="fop?"
    ).structured_content
    assert lower_case["items"] == []

    fopt = call_tool(
        rips_instance, "summary_values", case_id=case.id, vector="FOPT", max_points=4
    ).structured_content
    assert fopt["count"] == 60
    assert len(fopt["points"]) == 4
    assert fopt["points"][0]["time"].startswith("2015-04-09")
    assert fopt["points"][-1]["value"] == pytest.approx(51395.199, abs=1e-3)


def test_empty_project_lists(rips_instance, initialize_test):
    for name in ("list_views", "list_plots", "list_surfaces"):
        result = call_tool(rips_instance, name).structured_content
        assert result == {"total": 0, "truncated": False, "items": []}


def test_unknown_ids_are_tool_errors(rips_instance, initialize_test):
    for name, arguments, message in [
        ("case_info", {"case_id": 99}, "No grid case with id 99"),
        ("summary_values", {"case_id": 99, "vector": "FOPT"}, "No summary case"),
        ("snapshot_view", {"view_id": 99}, "No view with id 99"),
        ("snapshot_plot", {"plot_id": 99}, "No plot with id 99"),
    ]:
        result = call_tool(rips_instance, name, **arguments)
        assert result.is_error
        assert message in result.content[0].text


def test_snapshot(rips_instance, initialize_test):
    if not rips_instance.is_gui():
        pytest.skip("Cannot run test without a GUI")

    case = rips_instance.project.load_case(dataroot.PATH + GRID_CASE)
    view = case.create_view()
    summary_case = rips_instance.project.import_summary_case(
        dataroot.PATH + SUMMARY_CASE
    )
    collection = rips_instance.project.descendants(rips.SummaryPlotCollection)[0]
    collection.new_summary_plot(summary_cases=[summary_case], address="FOPT")
    plot = rips_instance.project.descendants(rips.MultiSummaryPlot)[-1]

    # Views and plots are numbered separately, so the ids can be equal
    for name, arguments in [
        ("snapshot_view", {"view_id": view.id}),
        ("snapshot_plot", {"plot_id": plot.id}),
    ]:
        result = call_tool(rips_instance, name, **arguments)
        # The export is a no-op when the window has no viewer (e.g. offscreen Qt platform)
        if not result.is_error:
            assert result.content[0].mime_type == "image/png"


def test_connection_launches_once(monkeypatch):
    # Tool calls run on worker threads, so the first calls can arrive together
    class LaunchedInstance:
        launched = True
        exited = False

        def check_alive(self) -> None:
            pass

        def exit(self) -> None:
            self.exited = True

    def find() -> Any:
        raise rips.RipsError("No ResInsight running")

    launched = []

    def launch() -> Any:
        time.sleep(0.2)
        launched.append(LaunchedInstance())
        return launched[-1]

    monkeypatch.setattr(rips.Instance, "find", find)
    monkeypatch.setattr(rips.Instance, "launch", launch)

    connection = _Connection(launch=True)
    threads = [threading.Thread(target=connection) for _ in range(4)]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()
    assert len(launched) == 1

    connection.close()
    assert launched[0].exited
