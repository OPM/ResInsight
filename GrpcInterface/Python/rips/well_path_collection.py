import uuid
import grpc
from typing import List, Optional, Union, cast

import SimulatorTables_pb2
import SimulatorTables_pb2_grpc

import PdmObject_pb2

from .pdmobject import add_method
from .resinsight_classes import WellPathCollection, WellPath
from .project import Project


@add_method(WellPathCollection)
def __custom_init__(
    self: WellPathCollection, pb2_object: PdmObject_pb2.PdmObject, channel: grpc.Channel
) -> None:
    self.__well_path_stub = SimulatorTables_pb2_grpc.WellPathStub(channel)


@add_method(WellPathCollection)
def import_well_path_from_points(
    self: WellPathCollection, name: str, coordinates: List[List[float]]
) -> object:
    """Create a well path from a list of XYZ coordinates.

    Arguments:
        name (str): Name of the well path
        coordinates (List[List[float]]): List of [x, y, z] coordinate triplets

    Returns:
        RimPointBasedWellPath: The created well path object

    Raises:
        ValueError: If coordinates are invalid or have wrong dimensions
    """

    if not name:
        raise ValueError("Name cannot be empty")

    if not coordinates:
        raise ValueError("Coordinates list cannot be empty")

    # Validate coordinate format
    for i, coord in enumerate(coordinates):
        if not isinstance(coord, (list, tuple)) or len(coord) != 3:
            raise ValueError(
                f"Coordinate at index {i} must be a list/tuple of 3 values [x, y, z]"
            )

        # Convert to float to validate numeric values
        try:
            float(coord[0])
            float(coord[1])
            float(coord[2])
        except (ValueError, TypeError):
            raise ValueError(
                f"All coordinate values must be numeric. Invalid coordinate at index {i}: {coord}"
            )

    # Extract separate x, y, z arrays
    x_coords = [float(coord[0]) for coord in coordinates]
    y_coords = [float(coord[1]) for coord in coordinates]
    z_coords = [float(coord[2]) for coord in coordinates]

    # Generate temporary keys with shared UUID for key-value store
    shared_uuid = uuid.uuid4()
    x_key = f"{shared_uuid}_coordinate_x"
    y_key = f"{shared_uuid}_coordinate_y"
    z_key = f"{shared_uuid}_coordinate_z"

    # Store coordinates in key-value store
    project = self.ancestor(Project)
    if not project:
        raise RuntimeError("Could not find parent project")

    project.set_key_values(x_key, x_coords)
    project.set_key_values(y_key, y_coords)
    project.set_key_values(z_key, z_coords)

    try:
        # Call internal method to create the well path
        well_path = self.import_well_path_from_points_internal(
            name=name,
            coordinate_x_key=x_key,
            coordinate_y_key=y_key,
            coordinate_z_key=z_key,
        )

        return well_path

    finally:
        # Clean up temporary keys from key-value store
        project.remove_key_values(x_key)
        project.remove_key_values(y_key)
        project.remove_key_values(z_key)


@add_method(WellPathCollection)
def completion_data_unified(
    self: WellPathCollection,
    wells: Optional[Union[List[WellPath], List[str]]] = None,
    case_id: Optional[int] = None,
) -> SimulatorTables_pb2.SimulatorTableData:
    """Get unified completion data for multiple wells.

    This method merges completion data (COMPDAT, WELSPECS, WELSEGS, COMPSEGS,
    WSEGVALV, WSEGAICD) from multiple wells into a single response.

    Arguments:
        wells: List of WellPath objects or well names. If None, uses all wells in collection.
        case_id: ID of the case to use. If None, tries to determine from context.

    Returns:
        SimulatorTableData containing merged completion data from all specified wells

    Raises:
        ValueError: If case_id cannot be determined or wells list is invalid
        RuntimeError: If unable to access required data
    """
    # Determine case_id if not provided
    if case_id is None:
        project = self.ancestor(Project)
        if not project:
            raise RuntimeError("Could not find parent project")

        # Try to get case_id from the first available case
        cases = project.cases()
        if not cases:
            raise RuntimeError("No cases available in project")
        case_id = cases[0].case_id

    # Handle wells parameter
    if wells is None:
        # Get all wells in collection
        well_names = [wp.name for wp in self.well_paths()]
    elif all(isinstance(w, str) for w in wells):
        # List of well names
        well_names = cast(List[str], wells)
    elif all(hasattr(w, "name") for w in wells):
        # List of WellPath objects
        well_names = [w.name for w in wells if hasattr(w, "name")]
    else:
        raise ValueError(
            "wells parameter must be a list of WellPath objects or strings"
        )

    if not well_names:
        raise ValueError("No wells specified or found in collection")

    sim_tab_req = SimulatorTables_pb2.SimulatorTableUnifiedRequest(
        wellpath_names=well_names, case_id=case_id
    )
    return self.__well_path_stub.GetCompletionDataUnified(sim_tab_req)
