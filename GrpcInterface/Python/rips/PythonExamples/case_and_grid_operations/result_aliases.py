#######################################################
#
# This file shows how to set up an alias name for
# a result, so that you could access the data
# using the alias name.
#
# This works for both eclipse and roff cases.
#
#######################################################


# Access to environment variables and path tools
import os
import pathlib

# Load ResInsight Processing Server Client Library
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()

# This requires the TestModels to be installed with ResInsight
# (RESINSIGHT_BUNDLE_TESTMODELS):
resinsight_exe_path = os.environ.get("RESINSIGHT_EXECUTABLE")

# Get the TestModels path from the executable path
resinsight_install_path = pathlib.PurePath(
    os.path.dirname(resinsight_exe_path)
).as_posix()

test_models_path = resinsight_install_path + "/TestModels/"

# Get the .roff case
roff_case_path = os.path.join(test_models_path, "reek/reek_box_grid_w_props.roff")

roff_case = resinsight.project.load_case(roff_case_path)

# Print all available properties
print("Results on file:")
for prop in roff_case.available_properties("STATIC_NATIVE"):
    print(prop)

# The name "DYBDE" should point to the "DEPTH" result
roff_case.add_result_alias("DEPTH", "DYBDE")

# The name "PERMX" should point to the "BOTTOM" result
roff_case.add_result_alias("BOTTOM", "PERMX")

# Print all available properties, now with two additional props
print("Results including aliases:")
for prop in roff_case.available_properties("STATIC_NATIVE"):
    print(prop)

real_result = roff_case.grid_property("STATIC_NATIVE", "BOTTOM", 0)
alias_result = roff_case.grid_property("STATIC_NATIVE", "PERMX", 0)

if real_result[40] == alias_result[40]:
    print("Result values at index 40 match!")

# Remove our aliases
roff_case.clear_result_aliases()

# Print all available properties, now just the original ones
print("Original results:")
for prop in roff_case.available_properties("STATIC_NATIVE"):
    print(prop)

try:
    alias_result = roff_case.grid_property("STATIC_NATIVE", "PERMX", 0)
except Exception:
    print("Result PERMX no longer exists!")
