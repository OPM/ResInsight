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
        PdmObject.__init__(self, pdm_object.pb2_object(), pdm_object.channel(), pdm_object.project())

    def export_snapshot(self, export_folder='', file_prefix='', output_format='PNG'):
        """ Export snapshot for the current plot
        
        Arguments:
            export_folder(str): The path to export to. By default will use the global export folder
            prefix (str): Exported file name prefix
            output_format(str): Enum string. Can be 'PNG' or 'PDF'.
            
        """
        return self._execute_command(
            exportSnapshots=Cmd.ExportSnapshotsRequest(type='PLOTS',
                                                       prefix=file_prefix,
                                                       viewId=self.id,
                                                       exportFolder=export_folder,
                                                       plotOutputFormat=output_format))

   