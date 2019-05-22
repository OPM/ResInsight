import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))
import ResInsight

resInsight     = ResInsight.Instance.find()

totalCellCount = resInsight.gridInfo.cellCount(caseId=0).reservoir_cell_count

values = []
for i in range(0, totalCellCount):
    values.append(i % 2 * 0.75);

print("Applying values to full grid")
resInsight.properties.setGridResults(values, 0, 'DYNAMIC_NATIVE', 'SOIL', 0)

