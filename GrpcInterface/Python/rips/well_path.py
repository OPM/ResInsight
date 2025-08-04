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
