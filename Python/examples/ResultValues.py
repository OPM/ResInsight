import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))
import ResInsight

resInsight     = ResInsight.Instance.find()
#gridCount      = resInsight.gridInfo.getGridCount(caseId=0)
#gridDimensions = resInsight.gridInfo.getAllGridDimensions(caseId=0)

resultChunks = resInsight.properties.activeCellResults(0, 'DYNAMIC_NATIVE', 'SOIL', 2)

results = []
for resultChunk in resultChunks:
	for value in resultChunk.values:
		results.append(value)
print("Number of active cells: " + str(len(results)))
print("15th active cell: ")
for result in results:
	print(result)
