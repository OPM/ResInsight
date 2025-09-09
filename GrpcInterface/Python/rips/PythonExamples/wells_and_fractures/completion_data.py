###################################################################################
# This example will connect to ResInsight, retrieve a list of wells, print info and
#  get the completion data for the first well found
#
###################################################################################

# Import the ResInsight Processing Server Module
import rips

# Connect to ResInsight
resinsight = rips.Instance.find()
if resinsight is None:
    exit(1)

# Get a list of all wells
wells = resinsight.project.well_paths()

# Get a list of all cases
cases = resinsight.project.cases()

# Use the first one
the_case = cases[0]
print("Using case " + the_case.name)

print("Got " + str(len(wells)) + " well paths: ")
for well in wells:
    print("Well path name: " + well.name)
    compdat = well.completion_data(the_case.id).compdat

    print("COMPDAT")

    for line in compdat:
        txt = "   "
        txt += line.well_name + "  "
        txt += str(line.grid_i) + "  "
        txt += str(line.grid_j) + "  "
        txt += str(line.upper_k) + "  "
        txt += str(line.lower_k) + "  "
        txt += line.open_shut_flag + "  "
        txt += str(line.saturation) + "  "
        txt += str(line.transmissibility) + "  "
        txt += str(line.diameter) + "  "
        txt += str(line.kh) + "  "
        txt += str(line.skin_factor) + "  "
        txt += str(line.d_factor) + "  "
        txt += "'%s'" % line.direction

        print(txt)

    print("/")
