import os
# Load ResInsight Processing Server Client Library
import rips
# Connect to ResInsight instance
resInsight = rips.Instance.find()

cases = resInsight.project.cases()

well_paths = resInsight.project.well_paths()
if len(well_paths) < 1:
    print("No well paths in project")
    exit(1)
print(well_paths)

for case in cases:
    if case.type == "GeoMechCase":
        print (case.case_id)
        case_path = case.grid_path()
        folder_name = os.path.dirname(case_path)
    
        # create a folder to hold the snapshots
        dirname = os.path.join(folder_name, 'snapshots')
        resInsight.set_export_folder(export_type='SNAPSHOTS', path=dirname)

        plot = case.create_well_bore_stability_plot(well_path=well_paths[0], time_step=0)
        plot.export_snapshots()
