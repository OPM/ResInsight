import os
import grpc

# Load ResInsight Processing Server Client Library
import rips
# Connect to ResInsight instance
resInsight = rips.Instance.find()

cases = resInsight.project.cases()

well_paths = resInsight.project.import_well_paths(well_path_folder='D:/Projects/ResInsight-regression-test/ModelData/Norne_LessWellPaths')
well_log_files = resInsight.project.import_well_log_files(well_log_folder='D:/Projects/ResInsight-regression-test/ModelData/Norne_PLT_LAS')

well_paths = resInsight.project.well_paths()

if len(well_paths) < 1:
    print("No well paths in project")
    exit(1)
print(well_paths)

wbs_parameters = rips.WbsParameters()
wbs_parameters.user_poisson_ratio = 0.412347
wbs_parameters.pore_pressure_outside_reservoir_source = "USER_DEFINED"
wbs_parameters.user_pp_outside_reservoir = 1.1
wbs_parameters.fg_shale_source = "PROPORTIONAL_TO_SH"
wbs_parameters.user_fg_shale = 1.13

for case in cases:
    if case.type == "GeoMechCase":
        print (case.case_id)
        case_path = case.grid_path()
        folder_name = os.path.dirname(case_path)
        case.import_formation_names(formation_files=['D:/Projects/ResInsight-regression-test/ModelData/norne/Norne_ATW2013.lyr'])

        # create a folder to hold the snapshots
        dirname = os.path.join(folder_name, 'snapshots')
        print("Exporting to: " + dirname)

        for well_path in well_paths:
            try:
                wbsplot = case.create_well_bore_stability_plot(well_path=well_path, time_step=0, wbs_parameters=wbs_parameters)
                replace_params = wbsplot.parameters()
                replace_params.user_poisson_ratio = 0.654321
                replace_params.user_fg_shale = 1.0321
                wbsplot.set_parameters(replace_params)
                wbsplot.set_depth_type("TRUE_VERTICAL_DEPTH_RKB")
                wbsplot.export_snapshot(export_folder=dirname)

            except grpc.RpcError as e:
                print("Error: ", e.details())
