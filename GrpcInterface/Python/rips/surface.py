import uuid

from .pdmobject import add_method
from .resinsight_classes import RegularSurface
from .project import Project


@add_method(RegularSurface)
def set_property(self, name, values):
    """Export snapshot for the current plot

    Arguments:
        export_folder(str): The path to export to. By default will use the global export folder
        prefix (str): Exported file name prefix
        output_format(str): Enum string. Can be 'PNG' or 'PDF'.

    """

    project = self.ancestor(Project)

    key = "{}_{}".format(uuid.uuid4(), "regular_surface_key")

    project.set_key_values(key, values)
    return self.set_property_from_key(
        name=name,
        value_key=key
    )

