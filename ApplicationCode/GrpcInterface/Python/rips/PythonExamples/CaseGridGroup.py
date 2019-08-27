import os
import rips

resInsight  = rips.Instance.find()

casePaths = []
casePaths.append("C:/Users/lindk/source/repos/ResInsight/TestModels/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID")
casePaths.append("C:/Users/lindk/source/repos/ResInsight/TestModels/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID")
for casePath in casePaths:
    assert os.path.exists(casePath), "You need to set valid case paths for this script to work"

caseGroup = resInsight.project.createGridCaseGroup(casePaths=casePaths)

caseGroup.printObjectInfo()
    
#statCases = caseGroup.statisticsCases()
#caseIds = []
#for statCase in statCases:
#    statCase.setValue("DynamicPropertiesToCalculate", ["SWAT"])
#    statCase.update()
#    caseIds.append(statCase.getValue("CaseId"))

resInsight.commands.computeCaseGroupStatistics(caseGroupId=caseGroup.groupId)

view = caseGroup.views()[0]
cellResult = view.cellResult()
cellResult.setValue("ResultVariable", "PRESSURE_DEV")
cellResult.update()
        