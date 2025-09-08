import rips

resinsight = rips.Instance.find()
if resinsight is None:
    raise RuntimeError("No running ResInsight instance found on the expected ports.")

# Create a modeled well path and add well path targets
# The coordinates are based on the Norne case

well_path_coll = resinsight.project.well_path_collection()
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

################################
## Create a lateral well path and append it to the main well path
################################

lateral_well = well_path_coll.add_new_object(rips.ModeledWellPath)
lateral_well.name = "Lateral-1"
lateral_well.update()

lateral_geometry = lateral_well.well_path_geometry()

# Only create geometry starting from the first target of the lateral
lateral_geometry.use_auto_generated_target_at_sea_level = False

reference_point = lateral_geometry.reference_point
reference_point[0] = 457550
reference_point[1] = 7322481
reference_point[2] = 2735
lateral_geometry.update()  # Commit updates back to ResInsight

coord = [0, 0, 0]
lateral_geometry.append_well_target(coord)

coord = [454.28, 250, -10]
target = lateral_geometry.append_well_target(coord)

coord = [1054.28, 250, -50]
target = lateral_geometry.append_well_target(coord)

# Append the lateral to the main well path
well_path.append_lateral(lateral_well)
