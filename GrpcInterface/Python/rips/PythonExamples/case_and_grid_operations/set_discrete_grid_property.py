######################################################################
# This script creates a discrete (integer) grid property for all grid
# cells in the first case in the project.
#
# A discrete property is visualized with a category legend, in contrast
# to the continuous legend used for floating point properties. Passing
# data_type="INTEGER" flags the property as discrete regardless of the
# property name (previously this required the name to end with "NUM").
######################################################################
import rips

resinsight = rips.Instance.find()

case = resinsight.project.case(case_id=0)
grid = case.grid()
grid_cell_count = grid.cell_count()
print("total cell count : " + str(grid_cell_count))

# Assign a simple region id based on IJK index.
values = []
for i in range(0, grid_cell_count):
    values.append(i % 4)

print("Applying discrete values to main grid")
case.set_grid_property(values, "STATIC_NATIVE", "MY_REGION", 0, data_type="INTEGER")
