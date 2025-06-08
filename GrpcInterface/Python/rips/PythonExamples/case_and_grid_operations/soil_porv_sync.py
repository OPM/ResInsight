##############################################################################
# This example will create a derived result for each time step synchronously
##############################################################################

import rips
import time

resinsight = rips.Instance.find()
start = time.time()
case = resinsight.project.cases()[0]

# Read the full porv result
porv_results = case.active_cell_property("STATIC_NATIVE", "PORV", 0)
time_step_info = case.time_steps()

for i in range(0, len(time_step_info)):
    # Read the full SOIl result for time step i
    soil_results = case.active_cell_property("DYNAMIC_NATIVE", "SOIL", i)

    # Generate the result by looping through both lists in order
    results = []
    for soil, porv in zip(soil_results, porv_results):
        results.append(soil * porv)

    # Send back result
    case.set_active_cell_property(results, "GENERATED", "SOILPORVSync", i)

end = time.time()
print("Time elapsed: ", end - start)

print("Transferred all results back")

view = case.views()[0].apply_cell_result("GENERATED", "SOILPORVSync")
