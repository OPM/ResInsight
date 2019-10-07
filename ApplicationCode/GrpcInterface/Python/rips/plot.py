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

    def export_snapshot(self, prefix='', export_folder=''):
        """ Export snapshot for the current plot
        
        Arguments:
            prefix (str): Exported file name prefix
            export_folder(str): The path to export to. By default will use the global export folder
        """
        return self._execute_command(
            exportSnapshots=Cmd.ExportSnapshotsRequest(type='PLOTS',
                                                       prefix=prefix,
                                                       viewId=self.view_id,
                                                       exportFolder=export_folder))