import os
# Load ResInsight Processing Server Client Library
import rips
# Connect to ResInsight instance
resInsight = rips.Instance.find()

cases = resInsight.project.cases()

well_paths = resInsight.project.import_well_paths(well_path_folder='D:/Projects/ResInsight-regression-test/ModelData/Norne_LessWellPaths')
well_log_files = resInsight.project.import_well_log_files(well_log_folder='D:/Projects/ResInsight-regression-test/ModelData/Norne_PLT_LAS')

if len(well_paths) < 1:
    print("No well paths in project")
    exit(1)
print(well_paths)

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
            wbsplot = case.create_well_bore_stability_plot(well_path=well_path, time_step=0)
            wbsplot.export_snapshot(export_folder=dirname)
