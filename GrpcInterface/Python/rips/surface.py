import uuid

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
