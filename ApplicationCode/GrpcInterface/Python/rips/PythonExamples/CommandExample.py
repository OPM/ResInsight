###############################################################################
# This example will run a few ResInsight command file commands
# .. which are exposed in the Python interface.
# Including setting time step, window size and export snapshots and properties
###############################################################################
import os
import tempfile
import rips

# Load instance
resInsight = rips.Instance.find()

# Run a couple of commands
resInsight.commands.setTimeStep(caseId=0, timeStep=3)
resInsight.commands.setMainWindowSize(width=800, height=500)

# Create a temporary directory which will disappear at the end of this script
# If you want to keep the files, provide a good path name instead of tmpdirname
with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
    print("Temporary folder: ", tmpdirname)
    
    # Set export folder for snapshots and properties
    resInsight.commands.setExportFolder(type='SNAPSHOTS', path=tmpdirname)
    resInsight.commands.setExportFolder(type='PROPERTIES', path=tmpdirname)
    
    # Export snapshots
    resInsight.commands.exportSnapshots()
    
    # Print contents of temporary folder
    print(os.listdir(tmpdirname))
    
    assert(len(os.listdir(tmpdirname)) > 0)
    case = resInsight.project.case(id=0)
    
    # Export properties in the view
    resInsight.commands.exportPropertyInViews(0, "3D View", 0)
    
    # Check that the exported file exists
    expectedFileName = case.name + "-" + str("3D_View") + "-" + "T3" + "-SOIL"
    fullPath = tmpdirname + "/" + expectedFileName
    assert(os.path.exists(fullPath))

