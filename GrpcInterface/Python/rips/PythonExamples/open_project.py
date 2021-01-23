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
path_name = os.path.join(test_models_path, "TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp")

# Open a project
resinsight.project.open(path_name)
