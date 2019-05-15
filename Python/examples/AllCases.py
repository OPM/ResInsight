import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))

resInsight  = ResInsight.Instance.find()
caseInfos = resInsight.ProjectInfo.AllCases(ResInsight.Empty())
		
print ("Got " + str(len(caseInfos.case_info)) + " cases: ")
for caseInfo in caseInfos.case_info:
	print(caseInfo.name)
