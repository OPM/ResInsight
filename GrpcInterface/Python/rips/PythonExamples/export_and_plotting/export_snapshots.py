############################################################################
# This script will export snapshots for two properties in every loaded case
# And put them in a snapshots folder in the same folder as the case grid
############################################################################
import os
import rips

# Load instance
resinsight = rips.Instance.find()
cases = resinsight.project.cases()

# Set main window size
resinsight.set_main_window_size(width=800, height=500)

n = 5  # every n-th time_step for snapshot
property_list = ["SOIL", "PRESSURE"]  # list of parameter for snapshot

print("Looping through cases")
for case in cases:
    print("Case name: ", case.name)
    print("Case id: ", case.id)
    # Get grid path and its folder name
    case_path = case.file_path
    folder_name = os.path.dirname(case_path)

    # create a folder to hold the snapshots
    dirname = os.path.join(folder_name, "snapshots")

    if os.path.exists(dirname) is False:
        os.mkdir(dirname)

    print("Exporting to folder: " + dirname)
    resinsight.set_export_folder(export_type="SNAPSHOTS", path=dirname)

    time_steps = case.time_steps()
    print("Number of time_steps: " + str(len(time_steps)))

    for view in case.views():
        for property in property_list:
            view.apply_cell_result(
                result_type="DYNAMIC_NATIVE", result_variable=property
            )
        for time_step in range(0, len(time_steps), 10):
            view.set_time_step(time_step=time_step)
            view.export_snapshot()
