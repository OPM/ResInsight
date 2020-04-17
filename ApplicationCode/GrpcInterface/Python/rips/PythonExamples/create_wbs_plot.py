import os
import grpc

# Load ResInsight Processing Server Client Library
import rips
# Connect to ResInsight instance
resInsight = rips.Instance.find()

# Get all GeoMech cases
cases = resInsight.project.descendants(rips.GeoMechCase)

# Get all well paths
well_paths = resInsight.project.well_paths()

# Ensure there's at least one well path
if len(well_paths) < 1:
    print("No well paths in project")
    exit(1)

# Create a set of WbsParameters
params = rips.WbsParameters()
params.user_poisson_ratio = 0.23456
params.user_ucs = 123

# Loop through all cases
for case in cases:
    assert(isinstance(case, rips.GeoMechCase))
    min_res_depth, max_res_depth = case.reservoir_depth_range()

    # Find a good output path
    case_path = case.file_path
    folder_name = os.path.dirname(case_path)

    # Import formation names
    case.import_formation_names(formation_files=['D:/Projects/ResInsight-regression-test/ModelData/norne/Norne_ATW2013.lyr'])

    # create a folder to hold the snapshots
    dirname = os.path.join(folder_name, 'snapshots')
    print("Exporting to: " + dirname)

    for well_path in well_paths[0:4]: # Loop through the first five well paths
        # Create plot with parameters
        wbsplot = case.create_well_bore_stability_plot(well_path=well_path.name, time_step=0, parameters=params)