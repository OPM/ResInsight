# Load ResInsight Processing Server Client Library
import rips
import tempfile

# Connect to ResInsight instance
resinsight = rips.Instance.find()

# Get the project
project = resinsight.project

# Find all the well bore stability plots in the project
wbsplots = project.descendants(rips.WellBoreStabilityPlot)

# Chose a sensible output folder
dirname = tempfile.gettempdir()

# Loop through all Well Bore Stability plots
for wbsplot in wbsplots:
    # Set depth type a parameter and export snapshot
    wbsplot.depth_type = "TRUE_VERTICAL_DEPTH_RKB"

    # Example of setting parameters for existing plots
    params = wbsplot.parameters()
    params.user_poisson_ratio = 0.12345
    params.update()
    wbsplot.update()
    wbsplot.export_snapshot(export_folder=dirname)
