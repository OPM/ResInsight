######################################################################
# This script sets values for all grid cells in the first case in the project
# The script is intended to be used for TEST10K_FLT_LGR_NNC.EGRID
# This grid case contains one LGR
######################################################################
import rips

resinsight = rips.Instance.find()

case = resinsight.project.case(case_id=0)
grid = case.grid()
grid_cell_count = grid.cell_count()
print("total cell count : " + str(grid_cell_count))

values = []
for i in range(0, grid_cell_count):
    values.append(i % 2 * 0.75)

# Assign value to IJK grid cell at (31, 53, 21)
grid = case.grid()
property_data_index = grid.property_data_index_from_ijk(31, 53, 21)
values[property_data_index] = 1.5

print("Applying values to main grid")
case.set_grid_property(values, "STATIC_NATIVE", "MY_DATA", 0)

values_from_ri = case.grid_property("STATIC_NATIVE", "MY_DATA", 0)
assert values[property_data_index] == values_from_ri[property_data_index]

# Get LGR grid as grid index 1
grid = case.grid(1)
grid_cell_count = grid.cell_count()
print("lgr cell count : " + str(grid_cell_count))

values = []
for i in range(0, grid_cell_count):
    values.append(i % 3 * 0.75)

print("Applying values to LGR grid")
case.set_grid_property(values, "STATIC_NATIVE", "MY_DATA", 0, 1)
values_from_ri = case.grid_property("STATIC_NATIVE", "MY_DATA", 0, 1)
