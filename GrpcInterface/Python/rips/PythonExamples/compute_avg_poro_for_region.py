###########################################################################################
# This example will calculate the average value of the porosity property for a specified
# region in the reservoir model. The region is identified by its EQLNUM value.
###########################################################################################

import rips

resinsight = rips.Instance.find()

cases = resinsight.project.cases()

time_step = 0
region_number = 2

for case in cases:
    sum_poro = 0.0
    cell_cout = 0

    eqlnum = case.active_cell_property("STATIC_NATIVE", "EQLNUM", time_step)
    poro = case.active_cell_property("STATIC_NATIVE", "PORO", time_step)
    if len(eqlnum) != len(poro):
        print("Size of eqlnum and poro is not identical.")
        break

    for i in range(len(eqlnum)):
        if eqlnum[i] == region_number:
            sum_poro += poro[i]
            cell_cout += 1

    # Calculate the average porosity for the specified region
    if cell_cout > 0:
        average_poro = sum_poro / cell_cout
        print(
            f"Case {case.id}: Cell count {cell_cout} Average PORO for region {region_number} = {average_poro}"
        )
