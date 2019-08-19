import rips
import time
import numpy as np

def createResult(soilChunks, porvChunks):
    for (soilChunk, porvChunk) in zip(soilChunks, porvChunks):
        resultChunk = []
        number = 0
        npSoilChunk = np.array(soilChunk.values)
        npPorvChunk = np.array(porvChunk.values)
        yield npSoilChunk + npPorvChunk


resInsight   = rips.Instance.find()
start = time.time()
case         = resInsight.project.case(id=0)
timeStepInfo = case.timeSteps()

porvChunks   = case.properties.activeCellPropertyAsync('STATIC_NATIVE', 'PORV', 0)
porvArray = []
for porvChunk in porvChunks:
    porvArray.append(porvChunk)

for i in range (0, len(timeStepInfo)):
    soilChunks = case.properties.activeCellPropertyAsync('DYNAMIC_NATIVE', 'SOIL', i)
    input_iterator = createResult(soilChunks, iter(porvArray))
    case.properties.setActiveCellPropertyAsync(input_iterator, 'GENERATED', 'SOILPORVAsync', i)

end = time.time()
print("Time elapsed: ", end - start)
        
print("Transferred all results back")