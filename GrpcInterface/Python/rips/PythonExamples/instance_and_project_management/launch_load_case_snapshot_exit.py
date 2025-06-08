# Access to environment variables
import os

# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.launch()

# This requires the TestModels to be installed with ResInsight (RESINSIGHT_BUNDLE_TESTMODELS):
resinsight_exe_path = os.environ.get("RESINSIGHT_EXECUTABLE")

# Get the TestModels path from the executable path
resinsight_install_path = os.path.dirname(resinsight_exe_path)
test_models_path = os.path.join(resinsight_install_path, "TestModels")
path_name = os.path.join(
    test_models_path, "TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
)

# Load an example case. Needs to be replaced with a valid path!
case = resinsight.project.load_case(path_name)

# Get a view
view1 = case.views()[0]

# Set the time step for view1 only
view1.set_time_step(time_step=2)

# Set cell result to SOIL
view1.apply_cell_result(result_type="DYNAMIC_NATIVE", result_variable="SOIL")

# Set export folder for snapshots and properties
resinsight.set_export_folder(export_type="SNAPSHOTS", path="e:/temp")
resinsight.set_export_folder(export_type="PROPERTIES", path="e:/temp")

# Export all snapshots
resinsight.project.export_snapshots()

# Export properties in the view
view1.export_property()

resinsight.exit()
