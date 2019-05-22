import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))
import ResInsight

resInsight     = ResInsight.Instance.find()
#gridCount      = resInsight.gridInfo.getGridCount(caseId=0)
#gridDimensions = resInsight.gridInfo.getAllGridDimensions(caseId=0)

values = []
for i in range(0, 11124):
    values.append(i % 2 * 0.5);

print("Applying all values to time step 0")
resInsight.properties.setActiveCellResults(values, 0, 'DYNAMIC_NATIVE', 'SOIL', 0)

