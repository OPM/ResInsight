######################################################################
# This script sets values for SOIL for all grid cells in the first case in the project
######################################################################
import rips

resinsight     = rips.Instance.find()

case = resinsight.project.case(case_id=0)
total_cell_count = case.cell_count().reservoir_cell_count

values = []
for i in range(0, total_cell_count):
    values.append(i % 2 * 0.75);

print("Applying values to full grid")
case.set_grid_property(values, 'DYNAMIC_NATIVE', 'SOIL', 0)

