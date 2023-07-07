import os
import rips

resinsight = rips.Instance.find()

# ResInsight includes some test models. Adjust this path to fit your system
test_model_path = "e:/gitroot-second/ResInsight/TestModels"

case_paths = []
case_paths.append(test_model_path + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID")
case_paths.append(test_model_path + "/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID")
case_paths.append(test_model_path + "/Case_with_10_timesteps/Real30/BRUGGE_0030.EGRID")
case_paths.append(test_model_path + "/Case_with_10_timesteps/Real40/BRUGGE_0040.EGRID")

for case_path in case_paths:
    assert os.path.exists(
        case_path
    ), "You need to set valid case paths for this script to work"

case_group = resinsight.project.create_grid_case_group(case_paths=case_paths)

cases = case_group.descendants(rips.EclipseCase)
print("Got " + str(len(cases)) + " cases: ")

for case in cases:
    time_step_info = case.time_steps()
    porv_results = case.active_cell_property("STATIC_NATIVE", "PORV", 0)

    for time_step_index in range(0, len(time_step_info)):
        pressure_results = case.active_cell_property(
            "DYNAMIC_NATIVE", "PRESSURE", time_step_index
        )

        results = []
        for pressure, porv in zip(pressure_results, porv_results):
            results.append(pressure * porv)

        # set the computed values in the case
        case.set_active_cell_property(
            results, "GENERATED", "PRESSURE_PORV", time_step_index
        )

    print(
        "Case id: " + str(case.id),
        "  Case name: " + case.name,
        " : Calculation complete",
    )


print("Transferred all results back to ResInsight")

# one of "GENERATED", "DYNAMIC_NATIVE", "STATIC_NATIVE", "IMPORTED"
# https://api.resinsight.org/en/main/rips.html#result-definition
property_type = "GENERATED"

property_name = "PRESSURE_PORV"

statistics_case = case_group.create_statistics_case()
statistics_case.set_source_properties(property_type, [property_name])
statistics_case.compute_statistics()

view = statistics_case.create_view()
statistics_property_name = property_name + "_MEAN"
view.apply_cell_result(
    result_type=property_type, result_variable=statistics_property_name
)
