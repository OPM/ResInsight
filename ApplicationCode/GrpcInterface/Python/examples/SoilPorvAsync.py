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



resInsight   = ResInsight.Instance.find()
case         = resInsight.project.case(id=0)
timeStepInfo = case.timeSteps()

porvChunks   = case.properties.activeCellProperty('STATIC_NATIVE', 'PORV', 0)
porvArray = []
for porvChunk in porvChunks:
    porvArray.append(porvChunk)

for i in range (0, len(timeStepInfo.dates)):
    soilChunks = case.properties.activeCellProperty('DYNAMIC_NATIVE', 'SOIL', i)
    input_iterator = createResult(soilChunks, iter(porvArray))
    case.properties.setActiveCellPropertyAsync(input_iterator, 'GENERATED', 'SOILPORVAsync', i)

print("Transferred all results back")