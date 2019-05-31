import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))
import ResInsight

def createResult(poroChunks, permxChunks):
    for (poroChunk, permxChunk) in zip(poroChunks, permxChunks):
        resultChunk = []
        for (poro, permx) in zip(poroChunk.values, permxChunk.values):
            resultChunk.append(poro * permx)
        yield resultChunk


resInsight     = ResInsight.Instance.find()
case = resInsight.project.case(id=0)

poroChunks = case.properties.activeCellProperty('STATIC_NATIVE', 'PORO', 0)
permxChunks = case.properties.activeCellProperty('STATIC_NATIVE', 'PERMX', 0)

case.properties.setActiveCellPropertyAsync(createResult(poroChunks, permxChunks), 'GENERATED', 'POROPERMXAS', 0)

print("Transferred all results back")