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
def add_well_log(
    self: WellPath,
    name: str,
    measured_depth: List[float],
    channel_data: Dict[str, List[float]],
) -> object:
    """Add imported well log data to well path.

    Arguments:
        name (str): Name of the well log.
        measured_depth (List[float]): Array of measured depth values.
        channel_data (Dict[str, List[float]]): Dictionary mapping channel names to their values.

    Returns:
        The created well log object.
    """

    if not name:
        raise ValueError("Name cannot be empty")

    if not measured_depth:
        raise ValueError("Measured depth list cannot be empty")

    if not isinstance(measured_depth, list):
        raise TypeError("Measured depth must be a list")

    if not channel_data:
        raise ValueError("Channel data dictionary cannot be empty")

    if not isinstance(channel_data, dict):
        raise TypeError("Channel data must be a dictionary")

    # Validate measured depth values are numeric
    try:
        float_measured_depth = [float(v) for v in measured_depth]
    except (ValueError, TypeError) as e:
        raise TypeError("All measured depth values must be numeric") from e

    # Validate channel data
    validated_channels = {}
    for channel_name, values in channel_data.items():
        if not channel_name:
            raise ValueError("Channel name cannot be empty")

        if not isinstance(values, list):
            raise TypeError(f"Values for channel '{channel_name}' must be a list")

        if not values:
            raise ValueError(
                f"Values list for channel '{channel_name}' cannot be empty"
            )

        if len(values) != len(measured_depth):
            raise ValueError(
                f"Channel '{channel_name}' has {len(values)} values but measured depth has {len(measured_depth)} values"
            )

        # Validate that all values are numeric
        try:
            validated_channels[channel_name] = [float(v) for v in values]
        except (ValueError, TypeError) as e:
            raise TypeError(
                f"All values in channel '{channel_name}' must be numeric"
            ) from e

    # Store data in key-value store and prepare for GRPC call
    project = self.ancestor(Project)
    if not project:
        raise RuntimeError("Could not find project")

    # Generate unique keys for data transfer
    shared_uuid = uuid.uuid4()
    measured_depth_key = f"{shared_uuid}_measured_depth"

    # Store measured depth
    project.set_key_values(measured_depth_key, float_measured_depth)

    # Store all channels and build channel mappings
    temp_keys = [measured_depth_key]
    channel_mappings = []

    try:
        for channel_name, values in validated_channels.items():
            channel_key = f"{shared_uuid}_channel_{channel_name}"
            project.set_key_values(channel_key, values)
            temp_keys.append(channel_key)
            channel_mappings.append(f"{channel_name}:{channel_key}")

        # Create comma-separated string for channel mappings
        channel_keys_csv = ",".join(channel_mappings)

        # Call the internal GRPC method
        well_log = self.add_well_log_internal(
            name=name,
            measured_depth_key=measured_depth_key,
            channel_keys_csv=channel_keys_csv,
        )
        return well_log
    except Exception as e:
        # Clean up all temporary keys on failure
        for temp_key in temp_keys:
            try:
                project.remove_key_values(temp_key)
            except Exception:
                pass  # Ignore cleanup errors
        raise RuntimeError(f"Failed to create well log: {str(e)}") from e
