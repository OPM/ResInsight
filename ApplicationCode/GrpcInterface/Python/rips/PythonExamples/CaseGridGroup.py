import os
import rips

resinsight  = rips.Instance.find()

case_paths = []
case_paths.append("C:/Users/lindk/source/repos/ResInsight/TestModels/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID")
case_paths.append("C:/Users/lindk/source/repos/ResInsight/TestModels/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID")
for casePath in case_paths:
    assert os.path.exists(casePath), "You need to set valid case paths for this script to work"

case_group = resinsight.project.create_grid_case_group(case_paths=case_paths)

case_group.print_object_info()
    
#stat_cases = caseGroup.statistics_cases()
#case_ids = []
#for stat_case in stat_cases:
#    stat_case.set_value("DynamicPropertiesToCalculate", ["SWAT"])
#    stat_case.update()
#    case_ids.append(stat_case.get_value("CaseId"))

resinsight.commands.compute_case_group_statistics(case_group_id=case_group.group_id)

view = case_group.views()[0]
cellResult = view.set_cell_result()
cellResult.set_value("ResultVariable", "PRESSURE_DEV")
cellResult.update()
        