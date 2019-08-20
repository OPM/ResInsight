############################################################################
# This example returns the currently selected cases in ResInsight
# Because running this script in the GUI takes away the selection
# This script does not run successfully from within the ResInsight GUI
# And will need to be run from the command line separately from ResInsight
############################################################################

import rips

resInsight  = rips.Instance.find()
if resInsight is not None:
    cases = resInsight.project.selectedCases()

    print ("Got " + str(len(cases)) + " cases: ")
    for case in cases:
        print(case.name)
        for property in case.properties.available('DYNAMIC_NATIVE'):
            print(property)


