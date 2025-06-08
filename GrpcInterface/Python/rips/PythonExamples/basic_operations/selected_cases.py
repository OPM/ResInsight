############################################################################
# This example returns the currently selected cases in ResInsight
# Because running this script in the GUI takes away the selection
# This script does not run successfully from within the ResInsight GUI
# And will need to be run from the command line separately from ResInsight
############################################################################

import rips

resinsight = rips.Instance.find()
if resinsight is not None:
    cases = resinsight.project.selected_cases()

    print("Got " + str(len(cases)) + " cases: ")
    for case in cases:
        print(case.name)
        for property in case.available_properties("DYNAMIC_NATIVE"):
            print(property)
