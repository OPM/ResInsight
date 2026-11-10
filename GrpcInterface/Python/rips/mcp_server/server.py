"""The MCP server and its tools."""

from __future__ import annotations

import fnmatch
import math
import tempfile
from collections.abc import Callable, Iterable, Iterator
from contextlib import contextmanager
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

import grpc
from mcp.server import MCPServer
from mcp.server.mcpserver import Image
from mcp.server.mcpserver.exceptions import ToolError
from mcp.types import ToolAnnotations

import rips

INSTRUCTIONS = (
    "Read-only access to the project open in ResInsight. Start with list_cases;"
    " grid cases and summary cases have separate ids. Cell properties and"
    " summary vectors are returned as statistics or a capped number of points."
)

READ_ONLY = ToolAnnotations(read_only_hint=True)

# Longest list a tool returns. Longer lists are cut and reported with their total.
MAX_ITEMS = 500

# Property types looked up by list_properties and property_statistics
PROPERTY_TYPES = ("DYNAMIC_NATIVE", "STATIC_NATIVE", "INPUT_PROPERTY", "GENERATED")


def statistics(values: Iterable[float]) -> dict[str, Any]:
    """Summarize values. Percentiles are plain percentiles: p10 is the value
    10 % of the defined values lie below."""
    all_values = list(values)
    defined = sorted(v for v in all_values if math.isfinite(v))
    count = len(defined)
    result: dict[str, Any] = {
        "count": count,
        "undefined_count": len(all_values) - count,
    }
    if count == 0:
        return result

    def percentile(fraction: float) -> float:
        position = fraction * (count - 1)
        lower = int(position)
        upper = min(lower + 1, count - 1)
        return defined[lower] + (defined[upper] - defined[lower]) * (position - lower)

    result.update(
        min=defined[0],
        max=defined[-1],
        mean=math.fsum(defined) / count,
        p10=percentile(0.1),
        p50=percentile(0.5),
        p90=percentile(0.9),
    )
    return result


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


def _available_properties(case: Any, property_type: str) -> list[str]:
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
                f"{t.year:04d}-{t.month:02d}-{t.day:02d}" for t in case.time_steps()
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
    def property_statistics(
        case_id: int, property: str, time_step: int = 0
    ) -> dict[str, Any]:
        """Statistics of a cell property over the active cells of a grid case.

        time_step is a zero-based index into the time steps of case_info, and is
        ignored for static properties.
        """
        with _reported():
            case = grid_case(case_id)
            for property_type in PROPERTY_TYPES:
                if property in _available_properties(case, property_type):
                    break
            else:
                raise ToolError(
                    f"Case {case_id} has no property '{property}'. See list_properties."
                )
            values = case.active_cell_property(property_type, property, time_step)
            return {
                "property": property,
                "property_type": property_type,
                "time_step": time_step,
                **statistics(values),
            }

    @server.tool(annotations=READ_ONLY)
    def list_wells(case_id: int) -> dict[str, Any]:
        """Simulation wells of a grid case. ResInsight reads them through a view
        of the case, so a case without a view lists none."""
        with _reported():
            case = grid_case(case_id)
            return {
                "view_count": len(case.views()),
                "simulation_wells": _capped(
                    [well.name for well in case.simulation_wells()]
                ),
            }

    @server.tool(annotations=READ_ONLY)
    def list_summary_vectors(case_id: int, pattern: str = "*") -> dict[str, Any]:
        """Summary vectors of a summary case. pattern is a wildcard such as
        'FOPT*' or 'WOPR:*'."""
        with _reported():
            addresses = summary_case(case_id).available_addresses().values
            return _capped(sorted(fnmatch.filter(addresses, pattern)))

    @server.tool(annotations=READ_ONLY)
    def summary_values(
        case_id: int, vector: str, max_points: int = 50
    ) -> dict[str, Any]:
        """Statistics of a summary vector, and at most max_points of its values
        spread evenly over time."""
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
                **statistics(values),
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
        """3D views in the project, with the grid case each one shows."""
        with _reported():
            views = []
            for case in project().cases():
                for view in case.views():
                    views.append(
                        {
                            "id": view.id,
                            "type": type(view).__name__,
                            "case_id": case.id,
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

    @server.tool(annotations=READ_ONLY)
    def snapshot(view_id: int) -> Image:
        """PNG image of a 3D view or a plot window, by the id from list_views or
        list_plots. Needs a ResInsight with a GUI."""
        with _reported():
            window = project().view(view_id) or project().plot(view_id)
            if window is None:
                raise ToolError(
                    f"No view or plot with id {view_id}. See list_views, list_plots."
                )
            with tempfile.TemporaryDirectory(prefix="rips_mcp_") as folder:
                window.export_snapshot(export_folder=folder)
                images = sorted(Path(folder).glob("*.png"))
                if not images:
                    raise ToolError(
                        "ResInsight wrote no snapshot, the window has no viewer."
                    )
                return Image(data=images[0].read_bytes(), format="png")

    return server
