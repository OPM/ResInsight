# Load ResInsight Processing Server Client Library
import rips
import time

# Connect to ResInsight instance
resinsight = rips.Instance.find()

# Create a modeled well path and add well path targets
# The coordinates are based on the Norne case
# Add a lateral to the main well path

well_path_coll = resinsight.project.descendants(rips.WellPathCollection)[0]
well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
well_path.name = "Test Well-1"
well_path.update()

geometry = well_path.well_path_geometry()

reference_point = geometry.reference_point
reference_point[0] = 457196
reference_point[1] = 7322270
reference_point[2] = 2742
geometry.update()  # Commit updates back to ResInsight

# Create the first well target at the reference point
coord = [0, 0, 0]
geometry.append_well_target(coord)

# Append new well targets relative the the reference point
coord = [454.28, 250, -10]
target = geometry.append_well_target(coord)

coord = [1054.28, 250, -50]
target = geometry.append_well_target(coord)

# Create a lateral at specified location on parent well
measured_depth = 3600
lateral = well_path.append_lateral(measured_depth)
geometry = lateral.well_path_geometry()

coord = [770, 280, 50]
target = geometry.append_well_target(coord)

coord = [1054.28, -100, 50]
target = geometry.append_well_target(coord)

coord = [2054.28, -100, 45]
target = geometry.append_well_target(coord)


# Wait 2 second
print("Wait 2 seconds ...")
time.sleep(2)
print("Move reference point of parent well")

geometry = well_path.well_path_geometry()
reference_point = geometry.reference_point
reference_point[2] += 50
geometry.update()  # Commit updates back to ResInsight
