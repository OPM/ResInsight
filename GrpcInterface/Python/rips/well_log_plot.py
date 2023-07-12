"""
ResInsight Well Log Plot plot module
"""

import Commands_pb2

from .plot import Plot
from .pdmobject import PdmObjectBase, add_method
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
) -> List[str]:
    """Export LAS file(s) for the current plot

    Arguments:
        export_folder(str): The path to export to. By default will use the global export folder
        file_prefix (str): Exported file name prefix
        export_tvdrkb(bool): Export in TVD-RKB format
        capitalize_file_names(bool): Make all file names upper case
        resample_interval(double): if > 0.0 the files will be resampled

    Returns:
        A list of files exported
    """
    res = self._execute_command(
        exportWellLogPlotData=Commands_pb2.ExportWellLogPlotDataRequest(
            exportFormat="LAS",
            viewId=self.id,
            exportFolder=export_folder,
            filePrefix=file_prefix,
            exportTvdRkb=export_tvdrkb,
            capitalizeFileNames=capitalize_file_names,
            resampleInterval=resample_interval,
            convertCurveUnits=convert_to_standard_units,
        )
    )
    return res.exportWellLogPlotDataResult.exportedFiles


@add_method(WellLogPlot)
def export_data_as_ascii(
    self: WellLogPlot,
    export_folder: str,
    file_prefix: str = "",
    capitalize_file_names: bool = False,
) -> List[str]:
    """Export LAS file(s) for the current plot

    Arguments:
        export_folder(str): The path to export to. By default will use the global export folder
        file_prefix (str): Exported file name prefix
        capitalize_file_names(bool): Make all file names upper case

    Returns:
        A list of files exported
    """
    res = self._execute_command(
        exportWellLogPlotData=Commands_pb2.ExportWellLogPlotDataRequest(
            exportFormat="ASCII",
            viewId=self.id,
            exportFolder=export_folder,
            filePrefix=file_prefix,
            exportTvdRkb=False,
            capitalizeFileNames=capitalize_file_names,
            resampleInterval=0.0,
        )
    )
    return res.exportWellLogPlotDataResult.exportedFiles
