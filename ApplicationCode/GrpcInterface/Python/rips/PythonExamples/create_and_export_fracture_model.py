# Load ResInsight Processing Server Client Library
import rips
# Connect to ResInsight instance
resinsight = rips.Instance.find()
# Example code
project = resinsight.project

# Find a well
well_path = project.well_path_by_name("Well-1")
print("well path:", well_path)
fracture_model_collection = project.descendants(rips.FractureModelCollection)[0]

measured_depth = 4100.0
fracture_model = fracture_model_collection.new_fracture_model(well_path=well_path, measured_depth=measured_depth)
