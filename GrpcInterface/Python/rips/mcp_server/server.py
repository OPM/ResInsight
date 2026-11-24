"""The MCP server and its tools."""

from __future__ import annotations

import fnmatch
import tempfile
from collections.abc import Callable, Iterator
from contextlib import contextmanager
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Literal, get_args

import grpc
from mcp.server import MCPServer
from mcp.server.mcpserver import Image
from mcp.server.mcpserver.exceptions import ToolError
from mcp.types import ToolAnnotations

import rips

INSTRUCTIONS = (
    "Access to the project open in ResInsight. Start with list_cases;"
    " grid cases and summary cases have separate ids, and so have views and"
    " plots. Summary vectors are returned as a capped number of points. The"
    " change tools set what a view shows, open a project or load a case;"
    " nothing is saved or deleted."
)

READ_ONLY = ToolAnnotations(read_only_hint=True)
CHANGE = ToolAnnotations(read_only_hint=False, destructive_hint=False)
# Opening a project closes the open one without saving it
REPLACES_PROJECT = ToolAnnotations(read_only_hint=False, destructive_hint=True)

# Longest list a tool returns. Longer lists are cut and reported with their total.
MAX_ITEMS = 500

# Property types looked up by list_properties and accepted by set_cell_result
PropertyType = Literal[
    rips.PropertyType.DYNAMIC_NATIVE,
    rips.PropertyType.STATIC_NATIVE,
    rips.PropertyType.INPUT_PROPERTY,
    rips.PropertyType.GENERATED,
]
PROPERTY_TYPES: tuple[rips.PropertyType, ...] = get_args(PropertyType)

# The 3D views. 2D intersection and seismic views are not scriptable: they come
# back with id -1 and without a case. Plots are numbered apart from views.
VIEW_TYPES = (rips.EclipseView, rips.GeoMechView)


def _capped(items: list[Any]) -> dict[str, Any]:
    return {
        "total": len(items),
        "truncated": len(items) > MAX_ITEMS,
        "items": items[:MAX_ITEMS],
    }


def _spread_indices(count: int, max_points: int) -> list[int]:
    """Up to max_points indices spread evenly over range(count), ends included."""
    if count <= max_points:
        return list(range(count))
    if max_points == 1:
        return [count - 1]
    step = (count - 1) / (max_points - 1)
    return [round(i * step) for i in range(max_points)]


def _available_properties(case: Any, property_type: rips.PropertyType) -> list[str]:
    try:
        return list(case.available_properties(property_type))
    except rips.RipsError as error:
        # ResInsight answers NOT_FOUND for a property type without properties
        if error.code == grpc.StatusCode.NOT_FOUND:
            return []
        raise


@contextmanager
def _reported() -> Iterator[None]:
    """Hand ResInsight errors to the client as a readable tool error."""
    try:
        yield
    except rips.RipsError as error:
        raise ToolError(error.details or str(error)) from error
    except grpc.RpcError as error:
        details = getattr(error, "details", None)
        message = details() if callable(details) else None
        raise ToolError(message or str(error)) from error


def create_server(connect: Callable[[], rips.Instance]) -> MCPServer:
    """Create the server.

    Arguments:
        connect: Returns the ResInsight instance to talk to. Called on every
            tool call, so it can connect late and reconnect.
    """
    server = MCPServer("ResInsight", instructions=INSTRUCTIONS)

    def project() -> Any:
        return connect().project

    def grid_case(case_id: int) -> Any:
        case = project().case(case_id)
        if case is None:
            raise ToolError(f"No grid case with id {case_id}. See list_cases.")
        return case

    def views_with_cases() -> list[tuple[Any, Any]]:
        views: list[Any] = [v for v in project().views() if isinstance(v, VIEW_TYPES)]
        return [(view, view.case()) for view in views]

    def view_by_id(view_id: int) -> Any:
        for view in project().views():
            if isinstance(view, VIEW_TYPES) and view.id == view_id:
                return view
        raise ToolError(f"No view with id {view_id}. See list_views.")

    def summary_case(case_id: int) -> Any:
        case = project().summary_case(case_id)
        if case is None:
            raise ToolError(f"No summary case with id {case_id}. See list_cases.")
        return case

    @server.tool(annotations=READ_ONLY)
    def instance_info() -> dict[str, Any]:
        """Versions of ResInsight and rips, and whether ResInsight has a GUI."""
        with _reported():
            instance = connect()
            return {
                "resinsight_version": instance.version_string(),
                "rips_version": instance.client_version_string(),
                "gui": instance.is_gui(),
                "port": instance.port,
            }

    @server.tool(annotations=READ_ONLY)
    def list_cases() -> dict[str, Any]:
        """Grid cases and summary cases in the project, with their ids."""
        with _reported():
            return {
                "grid_cases": _capped(
                    [
                        {"id": case.id, "name": case.name, "type": type(case).__name__}
                        for case in project().cases()
                    ]
                ),
                "summary_cases": _capped(
                    [
                        {"id": case.id, "name": case.short_name}
                        for case in project().summary_cases()
                    ]
                ),
            }

    @server.tool(annotations=READ_ONLY)
    def case_info(case_id: int) -> dict[str, Any]:
        """Type, file, grid dimensions, cell counts and time steps of a grid case."""
        with _reported():
            case = grid_case(case_id)
            cell_count = case.cell_count()
            grids = case.grids()
            dimensions = grids[0].dimensions() if grids else None
            time_steps = [
                datetime(
                    t.year,
                    t.month,
                    t.day,
                    t.hour,
                    t.minute,
                    t.second,
                    tzinfo=timezone.utc,
                ).isoformat()
                for t in case.time_steps()
            ]
            return {
                "id": case.id,
                "name": case.name,
                "type": type(case).__name__,
                "file_path": case.file_path,
                "grid_count": len(grids),
                "main_grid_dimensions": (
                    [dimensions.i, dimensions.j, dimensions.k] if dimensions else None
                ),
                "active_cell_count": cell_count.active_cell_count,
                "reservoir_cell_count": cell_count.reservoir_cell_count,
                "time_steps": _capped(time_steps),
            }

    @server.tool(annotations=READ_ONLY)
    def list_properties(case_id: int) -> dict[str, Any]:
        """Cell properties of a grid case, by property type."""
        with _reported():
            case = grid_case(case_id)
            return {
                property_type: _capped(_available_properties(case, property_type))
                for property_type in PROPERTY_TYPES
            }

    @server.tool(annotations=READ_ONLY)
    def list_wells(case_id: int) -> dict[str, Any]:
        """Simulation wells of a grid case. ResInsight reads them through a view
        of the case, so a case without a view lists none."""
        with _reported():
            case = grid_case(case_id)
            views = [
                view
                for view, view_case in views_with_cases()
                if view_case is not None and view_case.id == case.id
            ]
            # Case.simulation_wells() reads the wells of the first view the same way
            wells = views[0].descendants(rips.SimulationWell) if views else []
            return {
                "view_count": len(views),
                "simulation_wells": _capped([well.name for well in wells]),
            }

    @server.tool(annotations=READ_ONLY)
    def list_summary_vectors(case_id: int, pattern: str = "*") -> dict[str, Any]:
        """Summary vectors of a summary case. pattern is a wildcard such as
        'FOPT*' or 'WOPR:*'."""
        with _reported():
            addresses = summary_case(case_id).available_addresses().values
            return _capped(
                sorted(a for a in addresses if fnmatch.fnmatchcase(a, pattern))
            )

    @server.tool(annotations=READ_ONLY)
    def summary_values(
        case_id: int, vector: str, max_points: int = 50
    ) -> dict[str, Any]:
        """At most max_points values of a summary vector, spread evenly over
        time. count is the number of values the vector has."""
        if not 1 <= max_points <= MAX_ITEMS:
            raise ToolError(f"max_points must be between 1 and {MAX_ITEMS}.")
        with _reported():
            case = summary_case(case_id)
            if vector not in case.available_addresses().values:
                raise ToolError(
                    f"Summary case {case_id} has no vector '{vector}'."
                    " See list_summary_vectors."
                )
            values = case.summary_vector_values(vector).values
            times = case.available_time_steps().values
            return {
                "vector": vector,
                "count": len(values),
                "points": [
                    {
                        "time": datetime.fromtimestamp(
                            times[i], timezone.utc
                        ).isoformat(),
                        "value": values[i],
                    }
                    for i in _spread_indices(min(len(values), len(times)), max_points)
                ],
            }

    @server.tool(annotations=READ_ONLY)
    def list_views() -> dict[str, Any]:
        """3D views in the project, with the case each one shows."""
        with _reported():
            views = []
            for view, case in views_with_cases():
                views.append(
                    {
                        "id": view.id,
                        "type": type(view).__name__,
                        "case_id": case.id if case else None,
                        "current_time_step": view.current_time_step,
                    }
                )
            return _capped(views)

    @server.tool(annotations=READ_ONLY)
    def list_plots() -> dict[str, Any]:
        """Plot windows in the project."""
        with _reported():
            return _capped(
                [
                    {"id": plot.id, "type": type(plot).__name__}
                    for plot in project().plots()
                ]
            )

    @server.tool(annotations=READ_ONLY)
    def list_surfaces() -> dict[str, Any]:
        """Surfaces in the project, ensemble members left out."""
        with _reported():
            return _capped(
                [
                    {
                        "name": surface.surface_user_description,
                        "type": type(surface).__name__,
                        "depth_offset": surface.depth_offset,
                    }
                    for surface in project().surfaces()
                ]
            )

    def _snapshot(window: Any) -> Image:
        with tempfile.TemporaryDirectory(prefix="rips_mcp_") as folder:
            window.export_snapshot(export_folder=folder)
            images = sorted(Path(folder).glob("*.png"))
            if not images:
                raise ToolError(
                    "ResInsight wrote no snapshot, the window has no viewer."
                )
            return Image(data=images[0].read_bytes(), format="png")

    @server.tool(annotations=READ_ONLY)
    def snapshot_view(view_id: int) -> Image:
        """PNG image of a 3D view, by the id from list_views. Needs a ResInsight
        started with a GUI or with --headless."""
        with _reported():
            view = project().view(view_id)
            if view is None:
                raise ToolError(f"No view with id {view_id}. See list_views.")
            return _snapshot(view)

    @server.tool(annotations=READ_ONLY)
    def snapshot_plot(plot_id: int) -> Image:
        """PNG image of a plot window, by the id from list_plots. Needs a
        ResInsight started with a GUI or with --headless."""
        with _reported():
            plot = project().plot(plot_id)
            if plot is None:
                raise ToolError(f"No plot with id {plot_id}. See list_plots.")
            return _snapshot(plot)

    @server.tool(annotations=CHANGE)
    def set_time_step(view_id: int, index: int) -> dict[str, Any]:
        """Show a time step in a 3D view. index is zero-based, into the time
        steps from case_info."""
        with _reported():
            view_by_id(view_id).set_time_step(time_step=index)
            return {
                "view_id": view_id,
                "current_time_step": view_by_id(view_id).current_time_step,
            }

    @server.tool(annotations=CHANGE)
    def set_cell_result(
        view_id: int, property_type: PropertyType, property: str
    ) -> dict[str, Any]:
        """Color the cells of an Eclipse view by a property from
        list_properties."""
        with _reported():
            view = view_by_id(view_id)
            if isinstance(view, rips.GeoMechView):
                raise ToolError(
                    f"View {view_id} is a GeoMechView, set_cell_result takes an"
                    " EclipseView."
                )
            if property not in _available_properties(view.case(), property_type):
                raise ToolError(
                    f"The case of view {view_id} has no {property_type} property"
                    f" '{property}'. See list_properties."
                )
            view.apply_cell_result(property_type, property)
            cell_result = view_by_id(view_id).cell_result()
            return {
                "view_id": view_id,
                "property_type": str(cell_result.result_type),
                "property": cell_result.result_variable,
            }

    @server.tool(annotations=REPLACES_PROJECT)
    def open_project(path: str) -> dict[str, Any]:
        """Open a project file (.rsp) in ResInsight. The open project is closed
        without saving. path is a path on the machine that runs ResInsight."""
        with _reported():
            project().open(path)
            return list_cases()

    @server.tool(annotations=CHANGE)
    def load_case(path: str) -> dict[str, Any]:
        """Load a grid case (EGRID, GRID, GRDECL, ROFF) or a geomechanical case
        (ODB, INP, PVD) and create a 3D view of it. path is a path on the machine
        that runs ResInsight."""
        with _reported():
            case = project().load_case(path)
            view = case.create_view()
            return {
                "id": case.id,
                "name": case.name,
                "type": type(case).__name__,
                "view_id": view.id,
            }

    return server
