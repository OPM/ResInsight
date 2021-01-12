# Load ResInsight Processing Server Client Library
import rips
# Connect to ResInsight instance
resinsight = rips.Instance.find()

# Load an example case. Needs to be replaced with a valid path!
case = resinsight.project.load_case(
    "C:/Users/lindk/Projects/ResInsight/TestModels/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID")

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
