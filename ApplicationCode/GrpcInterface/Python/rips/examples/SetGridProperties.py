import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

resInsight     = rips.Instance.find()

case = resInsight.project.case(id=0)
totalCellCount = case.cellCount().reservoir_cell_count

values = []
for i in range(0, totalCellCount):
    values.append(i % 2 * 0.75);

print("Applying values to full grid")
case.properties.setGridProperty(values, 'DYNAMIC_NATIVE', 'SOIL', 0)

