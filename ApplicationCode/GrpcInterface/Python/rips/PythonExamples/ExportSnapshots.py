############################################################################
# This script will export snapshots for two properties in every loaded case
# And put them in a snapshots folder in the same folder as the case grid
############################################################################
import os
import rips

# Load instance
resInsight = rips.Instance.find()
cases = resInsight.project.cases()

# Set main window size
resInsight.commands.setMainWindowSize(width=800, height=500)

n = 5                            # every n-th timestep for snapshot
property_list = ['SOIL', 'PRESSURE'] # list of parameter for snapshot

print ("Looping through cases")
for case in cases:
    # Get grid path and its folder name
    casepath = case.grid_path()
    foldername = os.path.dirname(casepath)
    
    # create a folder to hold the snapshots
    dirname = os.path.join(foldername, 'snapshots')
        
    if os.path.exists(dirname) is False:
        os.mkdir(dirname)
    
    print ("Exporting to folder: " + dirname)
    resInsight.commands.setExportFolder(type='SNAPSHOTS', path=dirname)
   
    timeSteps = case.time_steps()
    tss_snapshot = range(0, len(timeSteps), n)
    print(case.name, case.id, 'Number of timesteps: ' + str(len(timeSteps)))
    print('Number of timesteps for snapshoting: ' + str(len(tss_snapshot)))
        
    view = case.views()[0]
    for property in property_list:
        view.applyCellResult(resultType='DYNAMIC_NATIVE', resultVariable=property)
        for ts_snapshot in tss_snapshot:
            resInsight.commands.setTimeStep(caseId = case.id, timeStep = ts_snapshot)        
            resInsight.commands.exportSnapshots(type='VIEWS', caseId=case.id)  # ‘ALL’, ‘VIEWS’ or ‘PLOTS’ default is 'ALL'
