"""
ResInsight 2d plot module
"""

import warnings

from .pdmobject import add_method
from .resinsight_classes import (
    PlotWindow as PlotWindow,
    Plot as Plot,
    SnapshotFileFormat,
)


@add_method(PlotWindow)
def export_snapshot(
    self,
    export_folder="",
    file_prefix="",
    output_format="PNG",
    width=-1,
    height=-1,
    prefix=None,
    file_format=None,
):
    """Export snapshot for the current plot

    Arguments:
        export_folder(str): The path to export to. By default will use the 'snapshots' folder next to the project file.
        file_prefix (str): Deprecated, use prefix. Exported file name prefix
        output_format(str): Deprecated, use file_format. Enum string. Can be 'PNG' or 'PDF'.
        width (int): The width of the exported snapshot. By default will use the existing size.
        height (int): The height of the exported snapshot. By default will use the existing size.
        prefix (str): Exported file name prefix
        file_format (SnapshotFileFormat): 'PNG' or 'PDF'

    """
    if file_prefix:
        warnings.warn(
            "PlotWindow.export_snapshot(file_prefix=...) is deprecated, use prefix=...",
            DeprecationWarning,
            stacklevel=3,
        )
    if output_format != "PNG":
        warnings.warn(
            "PlotWindow.export_snapshot(output_format=...) is deprecated, use file_format=...",
            DeprecationWarning,
            stacklevel=3,
        )
    if prefix is None:
        prefix = file_prefix
    if file_format is None:
        file_format = SnapshotFileFormat(output_format)

    self._call_pdm_method_void(
        "exportSnapshot",
        export_folder=export_folder,
        prefix=prefix,
        width=width,
        height=height,
        file_format=file_format,
    )
