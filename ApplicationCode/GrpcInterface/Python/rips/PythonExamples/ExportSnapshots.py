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
resinsight.commands.set_main_window_size(width=800, height=500)

n = 5                            # every n-th time_step for snapshot
property_list = ['SOIL', 'PRESSURE'] # list of parameter for snapshot

print ("Looping through cases")
for case in cases:
    # Get grid path and its folder name
    case_path = case.grid_path()
    folder_name = os.path.dirname(case_path)
    
    # create a folder to hold the snapshots
    dirname = os.path.join(folder_name, 'snapshots')
        
    if os.path.exists(dirname) is False:
        os.mkdir(dirname)
    
    print ("Exporting to folder: " + dirname)
    resinsight.commands.set_export_folder(type='SNAPSHOTS', path=dirname)
   
    time_steps = case.time_steps()
    tss_snapshot = range(0, len(time_steps), n)
    print(case.name, case.id, 'Number of time_steps: ' + str(len(time_steps)))
    print('Number of time_steps for snapshoting: ' + str(len(tss_snapshot)))
        
    view = case.views()[0]
    for property in property_list:
        view.applyCellResult(resultType='DYNAMIC_NATIVE', resultVariable=property)
        for ts_snapshot in tss_snapshot:
            resinsight.commands.set_time_step(case_id = case.id, time_step = ts_snapshot)        
            resinsight.commands.export_snapshots(type='VIEWS', case_id=case.id)  # ‘ALL’, ‘VIEWS’ or ‘PLOTS’ default is 'ALL'
