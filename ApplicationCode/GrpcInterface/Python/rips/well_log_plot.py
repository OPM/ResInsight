"""
ResInsight Well Log Plot plot module
"""

import rips.generated.Commands_pb2 as Cmd

from rips.plot import Plot
from rips.pdmobject import PdmObject
from rips.generated.pdm_objects import WellLogPlot
from rips.pdmobject import add_method

@add_method(WellLogPlot)
def export_data_as_las(self, export_folder, file_prefix='', export_tvdrkb=False, capitalize_file_names=False, resample_interval=0.0, convert_to_standard_units=False):
    """ Export LAS file(s) for the current plot
        
    Arguments:
        export_folder(str): The path to export to. By default will use the global export folder
        file_prefix (str): Exported file name prefix
        export_tvdrkb(bool): Export in TVD-RKB format
        capitalize_file_names(bool): Make all file names upper case
        resample_interval(double): if > 0.0 the files will be resampled
        
    Returns:
        A list of files exported
    """        
    res = self._execute_command(exportWellLogPlotData=Cmd.ExportWellLogPlotDataRequest(exportFormat='LAS',
                                                                                        viewId=self.id,
                                                                                        exportFolder=export_folder,
                                                                                        filePrefix=file_prefix,
                                                                                        exportTvdRkb=export_tvdrkb,
                                                                                        capitalizeFileNames=capitalize_file_names,
                                                                                        resampleInterval=resample_interval,
                                                                                        convertCurveUnits=convert_to_standard_units))
    return res.exportWellLogPlotDataResult.exportedFiles

@add_method(WellLogPlot)
def export_data_as_ascii(self, export_folder, file_prefix='', capitalize_file_names=False):
    """ Export LAS file(s) for the current plot
        
    Arguments:
        export_folder(str): The path to export to. By default will use the global export folder
        file_prefix (str): Exported file name prefix
        capitalize_file_names(bool): Make all file names upper case
        
    Returns:
        A list of files exported
    """
    res = self._execute_command(exportWellLogPlotData=Cmd.ExportWellLogPlotDataRequest(exportFormat='ASCII',
                                                                                        viewId=self.id,
                                                                                        exportFolder=export_folder,
                                                                                        filePrefix=file_prefix,
                                                                                        exportTvdRkb=False,
                                                                                        capitalizeFileNames=capitalize_file_names,
                                                                                        resampleInterval=0.0))
    return res.exportWellLogPlotDataResult.exportedFiles
