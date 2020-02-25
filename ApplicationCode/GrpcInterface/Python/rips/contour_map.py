"""
ResInsight 3d contour map module
"""
import rips.generated.Commands_pb2 as Cmd

from rips.pdmobject import PdmObject, add_method
from rips.view import View
from rips.generated.pdm_objects import EclipseContourMap, GeoMechContourMap

@add_method(EclipseContourMap)
def export_to_text(self, export_file_name='', export_local_coordinates=False, undefined_value_label="NaN", exclude_undefined_values=False):
    """ Export snapshot for the current view
        
    Arguments:
        export_file_name(str): The file location to store results in.
        export_local_coordinates(bool): Should we export local coordinates, or UTM.
        undefined_value_label(str): Replace undefined values with this label.
        exclude_undefined_values(bool): Skip undefined values.
    """
    return self._execute_command(
        exportContourMapToText=Cmd.ExportContourMapToTextRequest(
            exportFileName=export_file_name,
            exportLocalCoordinates=export_local_coordinates,
            undefinedValueLabel=undefined_value_label,
            excludeUndefinedValues=exclude_undefined_values,
            viewId=self.id))

@add_method(GeoMechContourMap)
def export_to_text(self, export_file_name='', export_local_coordinates=False, undefined_value_label="NaN", exclude_undefined_values=False):
    """ Export snapshot for the current view
        
    Arguments:
        export_file_name(str): The file location to store results in.
        export_local_coordinates(bool): Should we export local coordinates, or UTM.
        undefined_value_label(str): Replace undefined values with this label.
        exclude_undefined_values(bool): Skip undefined values.
    """
    return self._execute_command(
        exportContourMapToText=Cmd.ExportContourMapToTextRequest(
            exportFileName=export_file_name,
            exportLocalCoordinates=export_local_coordinates,
            undefinedValueLabel=undefined_value_label,
            excludeUndefinedValues=exclude_undefined_values,
            viewId=self.id))

