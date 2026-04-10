import uuid

from .exception import RipsError
from .pdmobject import add_method
from .project import Project
from .resinsight_classes import RegularSurface
from typing import List


@add_method(RegularSurface)
def set_property(self: RegularSurface, name: str, values: List[float]) -> None:
    """Sets a property on a regular surface.

    Arguments:
        name(str): Name of the property.
        values (List[float]): Values to set (float32).
          Should be of nx*ny size (see RegularSurface).
    """

    key = "{}_{}".format(uuid.uuid4(), "regular_surface_key")

    project = self.ancestor(Project)
    if project:
        project.set_key_values(key, values)
        self.set_property_from_key(name=name, value_key=key)


@add_method(RegularSurface)
def available_properties(self: RegularSurface) -> List[str]:
    """Gets the list of available property names on a regular surface.

    Returns:
        List[str]: Names of all properties set on this surface.
    """
    result = self.property_names()
    return result.values


@add_method(RegularSurface)
def get_property(self: RegularSurface, name: str) -> List[float]:
    """Gets a property from a regular surface.

    Arguments:
        name(str): Name of the property.

    Returns:
        List[float]: Values of the property (float32).
          Size is nx*ny (see RegularSurface).
    """
    key = "{}_{}".format(uuid.uuid4(), "regular_surface_key")

    project = self.ancestor(Project)
    if not project:
        raise RipsError("Could not find parent project")

    try:
        self.get_property_to_key(name=name, value_key=key)
        return project.key_values(key)
    finally:
        try:
            project.remove_key_values(key)
        except Exception:
            pass
