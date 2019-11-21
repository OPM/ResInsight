###############################################################################
# This example will show setting time step, window size and export snapshots and properties
###############################################################################
import os
import tempfile
import rips

# Load instance
resinsight = rips.Instance.find()

# Set window sizes
resinsight.set_main_window_size(width=800, height=500)
resinsight.set_plot_window_size(width=1000, height=1000)


# Retrieve first case
case = resinsight.project.cases()[0]

# Get a view
view1 = case.views()[0]

# Clone the view
view2 = view1.clone()

# Set the time step for view1 only
view1.set_time_step(time_step=2)

# Set cell result to SOIL
view1.apply_cell_result(result_type='DYNAMIC_NATIVE', result_variable='SOIL')


# Create a temporary directory which will disappear at the end of this script
# If you want to keep the files, provide a good path name instead of tmpdirname
with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
    print("Temporary folder: ", tmpdirname)
    
    # Set export folder for snapshots and properties
    resinsight.set_export_folder(export_type='SNAPSHOTS', path=tmpdirname)
    resinsight.set_export_folder(export_type='PROPERTIES', path=tmpdirname)
    
    # Export all snapshots
    resinsight.project.export_snapshots()
        
    assert(len(os.listdir(tmpdirname)) > 0)
    
    # Export properties in the view
    view1.export_property()

    # Check that the exported file exists
    expected_file_name = case.name + "-" + str("3D_View") + "-" + "T2" + "-SOIL"
    full_path = tmpdirname + "/" + expected_file_name

    # Print contents of temporary folder
    print(os.listdir(tmpdirname))

    assert(os.path.exists(full_path))

