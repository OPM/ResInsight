# Load ResInsight Processing Server Client Library
import rips
# Connect to ResInsight instance
resinsight = rips.Instance.find()
# Example code
print("ResInsight version: " + resinsight.version_string())

modeled_well_paths = resinsight.project.descendants(rips.ModeledWellPath)

for wellpath in modeled_well_paths:
    geometry = wellpath.well_path_geometry()
    geometry.print_object_info()
    reference_point = geometry.reference_point
    reference_point[0] += 100
    geometry.update()
    geometry.print_object_info()