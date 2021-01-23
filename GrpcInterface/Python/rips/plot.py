"""
ResInsight 2d plot module
"""
import Commands_pb2

from .pdmobject import add_method
from .resinsight_classes import PlotWindow, Plot


@add_method(PlotWindow)
def export_snapshot(self, export_folder="", file_prefix="", output_format="PNG"):
    """Export snapshot for the current plot

    Arguments:
        export_folder(str): The path to export to. By default will use the global export folder
        prefix (str): Exported file name prefix
        output_format(str): Enum string. Can be 'PNG' or 'PDF'.

    """
    return self._execute_command(
        exportSnapshots=Commands_pb2.ExportSnapshotsRequest(
            type="PLOTS",
            prefix=file_prefix,
            viewId=self.id,
            exportFolder=export_folder,
            plotOutputFormat=output_format,
        )
    )
