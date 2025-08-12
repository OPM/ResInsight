###################################################################################
# This example will connect to ResInsight, retrieve a list of wells, print info and
#  get the completion data for the first well found
#
###################################################################################

# Import the ResInsight Processing Server Module
import rips

# Connect to ResInsight
resinsight = rips.Instance.find()
if resinsight is not None:
    # Get a list of all wells
    wells = resinsight.project.well_paths()

    print("Got " + str(len(wells)) + " wells: ")
    for well in wells:
        print("Well name: " + well.name)

    if len(wells) > 0:
        well_path_coll = resinsight.project.well_path_collection()

        comp_data = well_path_coll.well_completions(wells[0].name)
