"""
ResInsight Well Log Plot plot module
"""

import warnings

from .pdmobject import add_method
from .resinsight_classes import WellLogPlot

from typing import List


@add_method(WellLogPlot)
def export_data_as_las(
    self: WellLogPlot,
    export_folder: str,
    file_prefix: str = "",
    export_tvdrkb: bool = False,
    capitalize_file_names: bool = False,
    resample_interval: float = 0.0,
    convert_to_standard_units: bool = False,
    export_tvd_rkb: bool = None,
) -> List[str]:
    """Export LAS file(s) for the current plot

    Arguments:
        export_folder(str): The path to export to. Must exist.
        file_prefix (str): Exported file name prefix
        export_tvdrkb(bool): Deprecated spelling of export_tvd_rkb
        capitalize_file_names(bool): Make all file names upper case
        resample_interval(double): if > 0.0 the files will be resampled
        convert_to_standard_units(bool): Convert curve units to standard units
        export_tvd_rkb(bool): Export in TVD-RKB format

    Returns:
        A list of files exported
    """
    if export_tvdrkb:
        warnings.warn(
            "WellLogPlot.export_data_as_las(export_tvdrkb=...) is deprecated, use export_tvd_rkb=...",
            DeprecationWarning,
            stacklevel=3,
        )
    if export_tvd_rkb is None:
        export_tvd_rkb = export_tvdrkb

    from .resinsight_classes import DataContainerString

    res = self._call_pdm_method_return_value(
        "exportDataAsLas",
        DataContainerString,
        export_folder=export_folder,
        file_prefix=file_prefix,
        export_tvd_rkb=export_tvd_rkb,
        capitalize_file_names=capitalize_file_names,
        resample_interval=resample_interval,
        convert_to_standard_units=convert_to_standard_units,
    )
    return list(res.values)


@add_method(WellLogPlot)
def export_data_as_ascii(
    self: WellLogPlot,
    export_folder: str,
    file_prefix: str = "",
    capitalize_file_names: bool = False,
) -> List[str]:
    """Export ASCII file for the current plot

    Arguments:
        export_folder(str): The path to export to. Must exist.
        file_prefix (str): Exported file name prefix
        capitalize_file_names(bool): Make all file names upper case

    Returns:
        A list of files exported
    """
    from .resinsight_classes import DataContainerString

    res = self._call_pdm_method_return_value(
        "exportDataAsAscii",
        DataContainerString,
        export_folder=export_folder,
        file_prefix=file_prefix,
        capitalize_file_names=capitalize_file_names,
    )
    return list(res.values)
