#######################################################
#
# This file shows how to import properties for a
# grid case created with .ROFFASC files
#
# Same procedure can also be used for .GRDECL files
#
#######################################################


# Access to environment variables and path tools
import os

# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()

# This requires the TestModels to be installed with ResInsight (RESINSIGHT_BUNDLE_TESTMODELS):
resinsight_exe_path = os.environ.get("RESINSIGHT_EXECUTABLE")

# Get the TestModels path from the executable path
resinsight_install_path = os.path.dirname(resinsight_exe_path)
test_models_path = os.path.join(resinsight_install_path, "TestModels")

# Get the .roff case
roff_case_path = os.path.join(
    test_models_path, "reek/reek_box_grid_w_out_props.roffasc"
)
roff_case = resinsight.project.load_case(roff_case_path)

# PORO and EQLNUM should not be among available properties yet
for prop in roff_case.available_properties("INPUT_PROPERTY"):
    print(prop)

# Import properties with file paths
poro_property_path = os.path.join(
    test_models_path, "reek/reek_box_PORO_property.roffasc"
)
eqlnum_property_path = os.path.join(
    test_models_path, "reek/reek_box_EQLNUM_property.roffasc"
)
roff_case.import_properties(file_names=[poro_property_path, eqlnum_property_path])

# PORO and EQLNUM should now be among available properties
for prop in roff_case.available_properties("INPUT_PROPERTY"):
    print(prop)
