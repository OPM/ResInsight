import uuid
from typing import List

from .pdmobject import add_method
from .resinsight_classes import WellPathCollection
from .project import Project


@add_method(WellPathCollection)
def import_fixed_trajectory_well_path(
    self: WellPathCollection, name: str, coordinates: List[List[float]]
) -> object:
    """Create a well path from a list of XYZ coordinates.

    Arguments:
        name (str): Name of the well path
        coordinates (List[List[float]]): List of [x, y, z] coordinate triplets

    Returns:
        RimFixedTrajectoryWellPath: The created well path object

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
        well_path = self.import_fixed_trajectory_well_path_internal(
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
