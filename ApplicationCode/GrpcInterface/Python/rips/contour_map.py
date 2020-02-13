"""
ResInsight 3d contour map module
"""
import rips.generated.Commands_pb2 as Cmd

from rips.pdmobject import PdmObject
from rips.view import View
from enum import Enum

class ContourMapType(Enum):
    ECLIPSE = 1
    GEO_MECH = 2

    @staticmethod
    def get_identifier(map_type):
        if map_type==ContourMapType.ECLIPSE:
            return "RimContourMapView"
        elif map_type==ContourMapType.GEO_MECH:
            return "RimGeoMechContourMapView"
        else:
            raise Exception("Unknown contour map type: must be ECLIPSE or GEO_MECH")

class ContourMap(View):
    """ResInsight contour map class

    Attributes:
        view_id(int): View Id corresponding to the View Id in ResInsight project.

    """
    def __init__(self, pdm_object, project, map_type):
        View.__init__(self, pdm_object, project)
        self.map_type = map_type


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
                viewId=self.view_id))
