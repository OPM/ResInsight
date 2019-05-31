import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))
import ResInsight

resInsight     = ResInsight.Instance.find()

activeCellCount = resInsight.case.cellCount(caseId=0).active_cell_count

values = []
for i in range(0, activeCellCount):
    values.append(i % 2 * 0.5);


timeSteps = resInsight.case.timeSteps(caseId=0)
for i in range(0, len(timeSteps.dates)):
	print("Applying values to all time step " + str(i))
	resInsight.properties.setActiveCellProperty(values, 0, 'DYNAMIC_NATIVE', 'SOIL', i)

