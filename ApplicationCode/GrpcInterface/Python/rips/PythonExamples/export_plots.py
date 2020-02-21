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
	plot.export_snapshot(export_folder=export_folder, output_format='PDF')
	well_log_plot = plot.cast(rips.WellLogPlot)
	if well_log_plot is not None:
		well_log_plot.export_data_as_las(export_folder=export_folder)
		well_log_plot.export_data_as_ascii(export_folder=export_folder)
