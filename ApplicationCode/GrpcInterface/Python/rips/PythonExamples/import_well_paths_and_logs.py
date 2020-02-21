# Load ResInsight Processing Server Client Library
import rips
# Connect to ResInsight instance
resInsight = rips.Instance.find()

well_paths = resInsight.project.import_well_paths(well_path_folder='D:/Projects/ResInsight-regression-test/ModelData/norne/wellpaths')
if resInsight.project.has_warnings():
    for warning in resInsight.project.warnings():
        print(warning)


for well_path in well_paths:
    print("Imported from folder: " + well_path.name)

well_paths = resInsight.project.import_well_paths(well_path_files=['D:/Projects/ResInsight-regression-test/ModelData/Norne_WellPaths/E-3H.json',
                                                                        'D:/Projects/ResInsight-regression-test/ModelData/Norne_WellPaths/C-1H.json'])
if resInsight.project.has_warnings():
    for warning in resInsight.project.warnings():
        print(warning)


for well_path in well_paths:
    print("Imported from individual files: " + well_path.name)


well_path_names = resInsight.project.import_well_log_files(well_log_folder='D:/Projects/ResInsight-regression-test/ModelData/Norne_PLT_LAS')
if resInsight.project.has_warnings():
    for warning in resInsight.project.warnings():
        print(warning)

for well_path_name in well_path_names:
    print("Imported well log file for: " + well_path_name)