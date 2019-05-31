import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))
import ResInsight

def createResult(soilChunks, porvChunks):
    for (soilChunk, porvChunk) in zip(soilChunks, porvChunks):
        resultChunk = []
        number = 0
        for (soil, porv) in zip(soilChunk.values, porvChunk.values):
            resultChunk.append(soil * porv)
            number += 1
        yield resultChunk



resInsight     = ResInsight.Instance.find()

timeStepInfo = resInsight.gridInfo.timeSteps(0)

porvChunks = resInsight.properties.activeCellResults(0, 'STATIC_NATIVE', 'PORV', 0)
porvArray = []
for porvChunk in porvChunks:
    porvArray.append(porvChunk)

for i in range (0, len(timeStepInfo.date)):
    soilChunks = resInsight.properties.activeCellResults(0, 'DYNAMIC_NATIVE', 'SOIL', i)
    input_iterator = createResult(soilChunks, iter(porvArray))
    resInsight.properties.setActiveCellResultsAsync(input_iterator, 0, 'GENERATED', 'SOILPORVAsync', i)

print("Transferred all results back")