import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))

import ResInsight

resInsight  = ResInsight.Instance.find()
if resInsight is not None:
    cases = resInsight.project.selectedCases()

    print ("Got " + str(len(cases)) + " cases: ")
    for case in cases:
        print(case.name)
        for property in case.properties.available('DYNAMIC_NATIVE'):
            print(property)


