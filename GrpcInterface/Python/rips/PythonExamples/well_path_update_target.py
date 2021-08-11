# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()

# Create a modeled well path and add well path targets
# The coordinates are fitted for the Norne case

well_path_coll = resinsight.project.descendants(rips.WellPathCollection)[0]
well_paths = well_path_coll.well_paths()

for w in well_paths:
    print (w.name)
    if w.name == "Test Well-1" :
        myWellPath = w


geometry = myWellPath.well_path_geometry()
for target in geometry.well_path_targets():
    target_point = target.target_point
    target_point[2] += 10
    target.update()



