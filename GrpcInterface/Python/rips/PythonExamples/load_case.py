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
case.create_view()

# Print out lots of information from the case object
print("Case id: " + str(case.id))
print("Case name: " + case.name)
print("Case type: " + case.__class__.__name__)
print("Case file name: " + case.file_path)
print("Case reservoir bounding box:", case.reservoir_boundingbox())

timesteps = case.time_steps()
for t in timesteps:
    print("Year: " + str(t.year))
    print("Month: " + str(t.month))
