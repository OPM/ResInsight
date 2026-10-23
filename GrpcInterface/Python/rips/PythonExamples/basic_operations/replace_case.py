# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()
# Example code
print("ResInsight version: " + resinsight.version_string())

case = resinsight.project.case(case_id=0)
case.replace_grid(
    new_grid_file="C:/Users/lindkvis/Projects/ResInsight/TestModels/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
)

# The project is reloaded, so retrieve the case object again
case = resinsight.project.case(case_id=0)
print("New case name: " + case.name)
