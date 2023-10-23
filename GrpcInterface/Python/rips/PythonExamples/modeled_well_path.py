# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()

# Create a modeled well path and add well path targets
# The coordinates are based on the Norne case

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

well_path.append_perforation_interval(3300, 3350, 0.2, 0.76)


# Update skin factor of the perforation
perforation_coll = well_path.completions().perforations()
perforation = perforation_coll.perforations()[0]
new_skin_factor = 0.9
print(
    "Changing perforation skin factor from {} to {}.".format(
        perforation.skin_factor, new_skin_factor
    )
)
perforation.skin_factor = new_skin_factor
perforation.update()
