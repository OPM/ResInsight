###################################################################################
# This example will connect to ResInsight, retrieve a list of wells, print info and
#  make a duplicate of the first well found, if any
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

        # will only work for editable well paths (ModeledWellPath)
        new_well = well.duplicate()
        print("New Well name: " + new_well.name)
