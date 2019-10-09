"""
ResInsight 2d plot module
"""
import rips.generated.Commands_pb2 as Cmd

from rips.pdmobject import PdmObject

class Plot(PdmObject):
    """ResInsight plot class

    Attributes:
        view_id(int): View Id corresponding to the View Id in ResInsight project.

    """
    def __init__(self, pdm_object):
        PdmObject.__init__(self, pdm_object.pb2_object(), pdm_object.channel())
        self.view_id = pdm_object.get_value("ViewId")

    def export_snapshot(self, export_folder='', file_prefix='', ):
        """ Export snapshot for the current plot
        
        Arguments:
            export_folder(str): The path to export to. By default will use the global export folder
            prefix (str): Exported file name prefix
            
        """
        return self._execute_command(
            exportSnapshots=Cmd.ExportSnapshotsRequest(type='PLOTS',
                                                       prefix=file_prefix,
                                                       viewId=self.view_id,
                                                       exportFolder=export_folder))

    def export_data_as_las(self, export_folder, file_prefix='', export_tvdrkb=False, capitalize_file_names=False, resample_interval=0.0):
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
                                                                                           viewId=self.view_id,
                                                                                           exportFolder=export_folder,
                                                                                           filePrefix=file_prefix,
                                                                                           exportTvdRkb=export_tvdrkb,
                                                                                           capitalizeFileNames=capitalize_file_names,
                                                                                           resampleInterval=resample_interval))
        return res.exportWellLogPlotDataResult.exportedFiles

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
                                                                                           viewId=self.view_id,
                                                                                           exportFolder=export_folder,
                                                                                           filePrefix=file_prefix,
                                                                                           exportTvdRkb=False,
                                                                                           capitalizeFileNames=capitalize_file_names,
                                                                                           resampleInterval=0.0))
        return res.exportWellLogPlotDataResult.exportedFiles