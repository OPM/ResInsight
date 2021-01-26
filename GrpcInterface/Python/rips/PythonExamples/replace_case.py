# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()
# Example code
print("ResInsight version: " + resinsight.version_string())

case = resinsight.project.case(case_id=0)
case.replace(
    new_grid_file="C:/Users/lindkvis/Projects/ResInsight/TestModels/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
)
