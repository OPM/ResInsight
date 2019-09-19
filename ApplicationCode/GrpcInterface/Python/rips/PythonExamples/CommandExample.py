###############################################################################
# This example will run a few ResInsight command file commands
# .. which are exposed in the Python interface.
# Including setting time step, window size and export snapshots and properties
###############################################################################
import os
import tempfile
import rips

# Load instance
resinsight = rips.Instance.find()

# Run a couple of commands
resinsight.commands.set_time_step(case_id=0, time_step=3)
resinsight.commands.set_main_window_size(width=800, height=500)

# Create a temporary directory which will disappear at the end of this script
# If you want to keep the files, provide a good path name instead of tmpdirname
with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
    print("Temporary folder: ", tmpdirname)
    
    # Set export folder for snapshots and properties
    resinsight.commands.set_export_folder(type='SNAPSHOTS', path=tmpdirname)
    resinsight.commands.set_export_folder(type='PROPERTIES', path=tmpdirname)
    
    # Export snapshots
    resinsight.commands.export_snapshots()
    
    # Print contents of temporary folder
    print(os.listdir(tmpdirname))
    
    assert(len(os.listdir(tmpdirname)) > 0)
    case = resinsight.project.case(id=0)
    
    # Export properties in the view
    resinsight.commands.export_property_in_views(0, "3D View", 0)
    
    # Check that the exported file exists
    expectedFileName = case.name + "-" + str("3D_View") + "-" + "T3" + "-SOIL"
    fullPath = tmpdirname + "/" + expectedFileName
    assert(os.path.exists(fullPath))

