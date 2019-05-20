import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))

import ResInsight

resInsight  = ResInsight.Instance.find()
if resInsight is not None:
    caseInfos = resInsight.projectInfo.selectedCases()

    print ("Got " + str(len(caseInfos)) + " cases: ")
    for caseInfo in caseInfos:
	    print(caseInfo.name)
		
