"""
ResInsight 2d plot module
"""
import rips.generated.Commands_pb2 as Cmd

from rips.pdmobject import PdmObject
from rips.generated.pdm_objects import PlotWindow, Plot
from rips.pdmobject import add_method

@add_method(PlotWindow)
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

   