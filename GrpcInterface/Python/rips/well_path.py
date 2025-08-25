import uuid

from .pdmobject import add_method
from .project import Project
from .resinsight_classes import WellPath

from typing import Dict, List


@add_method(WellPath)
def trajectory_properties(
    self: WellPath,
    resampling_interval: float,
) -> Dict[str, List[float]]:
    """Extracts properties from a well path trajectory.

    Arguments:
        resampling_interval (float): Interval in measured depth.

    Returns:
        Dict of properties.
    """

    if resampling_interval <= 0.0:
        raise ValueError("Resampling interval must be positive.")

    # Property names that will be used as both dict keys and temp key suffixes
    properties = [
        "coordinate_x",
        "coordinate_y",
        "coordinate_z",
        "measured_depth",
        "azimuth",
        "inclination",
        "dogleg",
    ]

    # Generate temporary keys with shared UUID
    shared_uuid = uuid.uuid4()
    temp_keys = {prop: f"{shared_uuid}_{prop}" for prop in properties}

    # Extract properties with temporary keys using **kwargs unpacking.
    # extract_well_path_properties puts results in the key-value store using
    # the specified keys.
    self.extract_well_path_properties_internal(
        resampling_interval=resampling_interval, **temp_keys
    )

    # Get the results from the key-value store.
    project = self.ancestor(Project)
    if project:
        result = {
            prop: project.key_values(temp_key) for prop, temp_key in temp_keys.items()
        }

        # Delete results form key-value store.
        for temp_key in temp_keys.values():
            project.remove_key_values(temp_key)

        return result
    else:
        return {}


@add_method(WellPath)
def add_well_log(self: WellPath, name: str, values: List[float]) -> object:
    """Add imported well log data to well path.

    Arguments:
        name (str): Name of the well log channel.
        values (List[float]): Array of well log values.

    Returns:
        The created well log object.
    """

    if not name:
        raise ValueError("Name cannot be empty")

    if not values:
        raise ValueError("Values list cannot be empty")

    if not isinstance(values, list):
        raise TypeError("Values must be a list")

    # Validate that all values are numeric
    try:
        float_values = [float(v) for v in values]
    except (ValueError, TypeError) as e:
        raise TypeError("All values must be numeric") from e

    # Generate temporary key for values
    temp_key = f"{uuid.uuid4()}_welllog_values"

    # Store values in key-value store
    project = self.ancestor(Project)
    if not project:
        raise RuntimeError("Could not find project")

    project.set_key_values(temp_key, float_values)

    try:
        # Call the internal GRPC method
        well_log = self.add_well_log_internal(name=name, values_key=temp_key)
        return well_log
    except Exception as e:
        # Clean up key-value store on failure
        project.remove_key_values(temp_key)
        raise RuntimeError(f"Failed to create well log: {str(e)}") from e
