import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))
import rips
import itertools

resInsight     = rips.Instance.find()
#gridCount      = resInsight.gridInfo.getGridCount(caseId=0)
#gridDimensions = resInsight.gridInfo.getAllGridDimensions(caseId=0)

poroChunks = resInsight.properties.activeCellResults(0, 'STATIC_NATIVE', 'PORO', 0)
permxChunks = resInsight.properties.activeCellResults(0, 'STATIC_NATIVE', 'PERMX', 0)

results = []
for (poroChunk, permxChunk) in zip(poroChunks, permxChunks):
    print("Received chunks")
    for (poro, permx) in zip(poroChunk.values, permxChunk.values):
        results.append(poro * permx)

print("Transferred " + str(len(results)) + " poro and permx results")

resInsight.properties.setActiveCellResults(results, 0, 'GENERATED', 'PORO*PERMX2', 0)
