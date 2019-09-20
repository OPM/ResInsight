###############################################################################
# This example will show setting time step, window size and export snapshots and properties
###############################################################################
import os
import tempfile
import rips

# Load instance
resinsight = rips.Instance.find()

# Set window size
resinsight.set_main_window_size(width=800, height=500)

# Retrieve first case
case = resinsight.project.cases()[0]

# Get a view
view1 = case.view(id=0)

# Clone the view
view2 = view1.clone()

# Set time step for all views
case.set_time_step_for_all_views(time_step=3)

# Set the time step for view1 only
view1.set_time_step(time_step=2)

# Create a temporary directory which will disappear at the end of this script
# If you want to keep the files, provide a good path name instead of tmpdirname
with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
    print("Temporary folder: ", tmpdirname)
    
    # Set export folder for snapshots and properties
    resinsight.set_export_folder(type='SNAPSHOTS', path=tmpdirname)
    resinsight.set_export_folder(type='PROPERTIES', path=tmpdirname)
    
    # Export all snapshots
    resinsight.project.export_snapshots()
    
    # Print contents of temporary folder
    print(os.listdir(tmpdirname))
    
    assert(len(os.listdir(tmpdirname)) > 0)
    
    # Export properties in the view
    case.properties.export_in_views(view_ids=0)

    # Check that the exported file exists
    expected_file_name = case.name + "-" + str("3D_View") + "-" + "T2" + "-SOIL"
    full_path = tmpdirname + "/" + expected_file_name
    print(full_path)
    assert(os.path.exists(full_path))

