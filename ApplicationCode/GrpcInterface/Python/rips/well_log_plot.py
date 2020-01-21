"""
ResInsight Well Log Plot plot module
"""

import rips.generated.Commands_pb2 as Cmd

from rips.plot import Plot
from rips.pdmobject import PdmObject

class WellLogPlot(Plot):
    """ResInsight well log plot class
    """

    def __init__(self, pdm_object):
        Plot.__init__(self, pdm_object)

    @classmethod
    def from_pdm_object(cls, pdm_object):
        if isinstance(pdm_object, PdmObject):
            if pdm_object.class_keyword() == "WellLogPlot":
                return cls(pdm_object)
        return None            

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

    def depth_range(self):
        """Get the depth range of the Plot
        """
        return self.get_value("MinimumDepth"), self.get_value("MaximumDepth")

    def set_depth_range(self, min_depth, max_depth, update=True):
        """ Set the visible depth range minimum

        Arguments:
            min_depth(double): The new minimum depth
            max_depth(double): The new maximum depth
            update(bool, optional): Update the plot after setting the value?
        """

        self.set_value("MinimumDepth", min_depth)
        self.set_value("MaximumDepth", max_depth)
        self.set_value("AutoScaleDepthEnabled", False)
        if update:
            self.update()

    def depth_type(self):
        """Get the plot depth type
        
        Returns: an enum string. Can be "MEASURED_DEPTH", "TRUE_VERTICAL_DEPTH" or "TRUE_VERTICAL_DEPTH_RKB".
        """
        return self.get_value("DepthType")

    def set_depth_type(self, depth_type, update=True):
        """Set the depth type

        Arguments:
            depth_type(enum string): can be "MEASURED_DEPTH", "TRUE_VERTICAL_DEPTH" or "TRUE_VERTICAL_DEPTH_RKB".
            update(bool, optional): Update the plot after setting the value?
        """
        self.set_value("DepthType", depth_type)
        if update:
            self.update()

    def depth_unit(self):
        """Get the plot depth units

        Returns: an enum string. Can be "UNIT_METER", "UNIT_FEET" or "UNIT_NONE".
        """

    def set_depth_unit(self, depth_unit, update=True):
        """Set the depth unit

        Arguments:
            depth_unit(enum string): can be "UNIT_METER", "UNIT_FEET" or "UNIT_NONE".
            update(bool, optional): Update the plot after setting the value?
        """
        self.set_value("DepthUnit", depth_unit)
        if update:
            self.update()
