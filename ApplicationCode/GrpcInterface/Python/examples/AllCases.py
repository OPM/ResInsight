import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))

import ResInsight

resInsight  = ResInsight.Instance.find()
if resInsight is not None:
    cases = resInsight.project.allCases()

    print ("Got " + str(len(cases)) + " cases: ")
    for case in cases:
        print(case.name)
