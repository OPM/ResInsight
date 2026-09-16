"""
ResInsight 3d contour map module
"""

from .pdmobject import add_method
from .view import View as View
from .resinsight_classes import EclipseContourMap, GeoMechContourMap


def export_to_text(
    self: View,
    export_file_name: str = "",
    export_local_coordinates: bool = False,
    undefined_value_label: str = "NaN",
    exclude_undefined_values: bool = False,
) -> None:
    """Export the contour map to a text file

    Alias of View.export_contour_map_to_text().

    Arguments:
        export_file_name(str): The file location to store results in.
        export_local_coordinates(bool): Should we export local coordinates, or UTM.
        undefined_value_label(str): Replace undefined values with this label.
        exclude_undefined_values(bool): Skip undefined values.
    """
    self.export_contour_map_to_text(
        export_file_name=export_file_name,
        export_local_coordinates=export_local_coordinates,
        undefined_value_label=undefined_value_label,
        exclude_undefined_values=exclude_undefined_values,
    )


add_method(EclipseContourMap)(export_to_text)
add_method(GeoMechContourMap)(export_to_text)
