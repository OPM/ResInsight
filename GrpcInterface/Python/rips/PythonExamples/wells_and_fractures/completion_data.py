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
print("Got " + str(len(cases)) + " cases: ")
for ecase in cases:
    print("Case name: " + ecase.name)

# Use the first one
the_case = cases[0]
print(the_case.id)
print(the_case)

print("Got " + str(len(wells)) + " well paths: ")
for well in wells:
    print("Well path name: " + well.name)
    compdat = well.completion_data(the_case.id)
    print(compdat)
    print(dir(compdat))


# if len(wells) > 0:
#    well_path_coll = resinsight.project.well_path_collection()
#
#    comp_data = well_path_coll.well_completions(wells[0].name, the_case.id)
#    print(comp_data.well_name)
#    print(comp_data)

#    cdata = comp_data.compdat()

#    if len(cdata) == 0:
#        print("No COMPDAT data found for well: " + wells[0].name)

#    for line in cdata:
#        print(line)
