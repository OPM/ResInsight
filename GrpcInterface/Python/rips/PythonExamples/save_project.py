# Access to environment variables and path tools
import os

# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()

# This requires the TestModels to be installed with ResInsight (RESINSIGHT_BUNDLE_TESTMODELS):
resinsight_exe_path = os.environ.get("RESINSIGHT_EXECUTABLE")

# Get the TestModels path from the executable path
resinsight_install_path = os.path.dirname(resinsight_exe_path)
test_models_path = os.path.join(resinsight_install_path, "TestModels")
path_name = os.path.join(
    test_models_path, "TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
)
case = resinsight.project.load_case(path_name)

# Save the project to file
home_dir = os.path.expanduser("~")
project_path = home_dir + "/new-project.rsp"
print("Saving project to: ", project_path)
resinsight.project.save(project_path)
