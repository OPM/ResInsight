import os
import rips

resinsight  = rips.Instance.find()

case_paths = []
case_paths.append("C:/Users/lindk/source/repos/ResInsight/TestModels/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID")
case_paths.append("C:/Users/lindk/source/repos/ResInsight/TestModels/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID")
for case_path in case_paths:
    assert os.path.exists(case_path), "You need to set valid case paths for this script to work"

case_group = resinsight.project.create_grid_case_group(case_paths=case_paths)

case_group.print_object_info()
    
#stat_cases = caseGroup.statistics_cases()
#case_ids = []
#for stat_case in stat_cases:
#    stat_case.set_value("DynamicPropertiesToCalculate", ["SWAT"])
#    stat_case.update()
#    case_ids.append(stat_case.get_value("CaseId"))

case_group.compute_statistics()

view = case_group.views()[0]
cell_result = view.set_cell_result()
cell_result.set_value("ResultVariable", "PRESSURE_DEV")
cell_result.update()
        