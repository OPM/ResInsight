import os
import rips

resinsight  = rips.Instance.find()

casePaths = []
casePaths.append("C:/Users/lindk/source/repos/ResInsight/TestModels/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID")
casePaths.append("C:/Users/lindk/source/repos/ResInsight/TestModels/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID")
for casePath in casePaths:
    assert os.path.exists(casePath), "You need to set valid case paths for this script to work"

caseGroup = resinsight.project.create_grid_case_group(casePaths=casePaths)

caseGroup.print_object_info()
    
#statCases = caseGroup.statistics_cases()
#caseIds = []
#for statCase in statCases:
#    statCase.set_value("DynamicPropertiesToCalculate", ["SWAT"])
#    statCase.update()
#    caseIds.append(statCase.get_value("CaseId"))

resinsight.commands.compute_case_group_statistics(caseGroupId=caseGroup.groupId)

view = caseGroup.views()[0]
cellResult = view.cellResult()
cellResult.set_value("ResultVariable", "PRESSURE_DEV")
cellResult.update()
        