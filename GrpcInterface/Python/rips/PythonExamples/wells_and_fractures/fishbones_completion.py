# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()

# Create a modeled well path and add well path targets
# The coordinates are based on the Norne case
# Add a lateral to the main well path

well_path_coll = resinsight.project.well_path_collection()

editable_well_paths = well_path_coll.descendants(rips.ModeledWellPath)
if editable_well_paths:
    # Use the first well path if it exists
    well_path = editable_well_paths[0]
else:
    # Create a well path
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "Test Well-1"
    well_path.update()

    geometry = well_path.well_path_geometry()

    reference_point = geometry.reference_point
    reference_point[0] = 457196
    reference_point[1] = 7322270
    reference_point[2] = 2742
    geometry.update()

    coord = [0, 0, 0]
    geometry.append_well_target(coord)

    coord = [454.28, 250, -10]
    target = geometry.append_well_target(coord)

    coord = [1054.28, 250, -50]
    target = geometry.append_well_target(coord)


# Add a fishbone completion to the well path with default settings
well_path.append_fishbones([4000.0, 4100.0, 4200.0])

# Optional settings on the collection level that will affect all installed fishbones
fishbones_collection = well_path.completions().fishbones()
fishbones_collection.main_bore_skin_factor = 0.1
fishbones_collection.update()

# Drilling type is one of [STANDARD, EXTENDED, ACID_JETTING]
drilling_type = "ACID_JETTING"

sub_locations = [3500.0, 3550.0, 3600.0, 3700.0]
fishbones = fishbones_collection.append_fishbones(sub_locations, drilling_type)

fishbones.lateral_diameter = 47.0
fishbones.lateral_skin_factor = 27.0
fishbones.update()

# Optionally set the fixed start location for all fishbones
fishbones_collection.set_fixed_start_location(2900.0)

# Export completions
cases = resinsight.project.cases()

for case in cases:
    print("Case name: ", case.name)
    print("Case id: ", case.id)

    case.export_well_path_completions(
        time_step=0,
        well_path_names=["Test Well-1 Y1"],
        file_split="UNIFIED_FILE",
        include_perforations=True,
        custom_file_name="f:/scratch/2023-11-02/myfile.myext",
    )
