import rips
import time

resInsight = rips.Instance.find()
start = time.time()
case       = resInsight.project.case(id=0)

porvResults = case.properties.activeCellProperty('STATIC_NATIVE', 'PORV', 0)
timeStepInfo = case.timeSteps()

for i in range (0, len(timeStepInfo)):
    soilResults = case.properties.activeCellProperty('DYNAMIC_NATIVE', 'SOIL', i)
    results = []
    for (soil, porv) in zip(soilResults, porvResults):
        results.append(soil * porv)

    case.properties.setActiveCellProperty(results, 'GENERATED', 'SOILPORVSync', i)

end = time.time()
print("Time elapsed: ", end - start)

print("Transferred all results back")