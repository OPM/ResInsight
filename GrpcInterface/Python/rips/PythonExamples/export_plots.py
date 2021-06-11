# Import the tempfile module
import tempfile

# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resInsight = rips.Instance.find()

# Get a list of all plots
plots = resInsight.project.plots()

export_folder = tempfile.mkdtemp()

print("Exporting to: " + export_folder)

for plot in plots:
    plot.export_snapshot(export_folder=export_folder)
    plot.export_snapshot(export_folder=export_folder, output_format="PDF")
    if isinstance(plot, rips.WellLogPlot):
        plot.export_data_as_las(export_folder=export_folder)
        plot.export_data_as_ascii(export_folder=export_folder)
