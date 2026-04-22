import uuid
from typing import Dict, List, Optional

from .exception import RipsError
from .pdmobject import add_method
from .project import Project
from .resinsight_classes import WellLog


def _try_read_key(project: Project, key: str) -> Optional[List[float]]:
    try:
        return project.key_values(key)
    except RipsError:
        return None


@add_method(WellLog)
def channel_names(self: WellLog) -> List[str]:
    """Get the list of channel names for this well log.

    Returns:
        A list of channel name strings.
    """
    return self.channel_names_internal().values


@add_method(WellLog)
def well_log_data(self: WellLog) -> Dict[str, List[float]]:
    """Read all well log data (measured depth, optional TVD arrays, and channel values).

    Returns:
        A dictionary with:
          - "measured_depth" (always present): List of measured depth values.
          - "tvd_msl" (only if the well log has TVD MSL values).
          - "tvd_rkb" (only if the well log has TVD RKB values).
          - One entry per channel name in `channel_names()`.
    """
    project = self.ancestor(Project)
    if project is None:
        raise RipsError("Could not find parent project")

    names = self.channel_names()

    shared_uuid = uuid.uuid4()
    md_key = f"{shared_uuid}_md"
    tvd_msl_key = f"{shared_uuid}_tvd_msl"
    tvd_rkb_key = f"{shared_uuid}_tvd_rkb"
    channel_keys = {name: f"{shared_uuid}_ch_{name}" for name in names}
    channel_keys_csv = ",".join(f"{name}:{key}" for name, key in channel_keys.items())

    try:
        self.read_well_log_data_internal(
            measured_depth_key=md_key,
            tvd_msl_key=tvd_msl_key,
            tvd_rkb_key=tvd_rkb_key,
            channel_keys_csv=channel_keys_csv,
        )

        result: Dict[str, List[float]] = {
            "measured_depth": project.key_values(md_key),
        }

        tvd_msl = _try_read_key(project, tvd_msl_key)
        if tvd_msl:
            result["tvd_msl"] = tvd_msl

        tvd_rkb = _try_read_key(project, tvd_rkb_key)
        if tvd_rkb:
            result["tvd_rkb"] = tvd_rkb

        for name, key in channel_keys.items():
            result[name] = project.key_values(key)

        return result
    finally:
        cleanup_keys = [md_key, tvd_msl_key, tvd_rkb_key, *channel_keys.values()]
        for key in cleanup_keys:
            try:
                project.remove_key_values(key)
            except Exception:
                pass
