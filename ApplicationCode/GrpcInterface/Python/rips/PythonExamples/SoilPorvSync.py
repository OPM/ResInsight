##############################################################################
# This example will create a derived result for each time step synchronously
##############################################################################

import rips
import time

resInsight = rips.Instance.find()
start = time.time()
case       = resInsight.project.case(id=0)

# Read the full porv result
porvResults = case.properties.activeCellProperty('STATIC_NATIVE', 'PORV', 0)
timeStepInfo = case.time_steps()

for i in range (0, len(timeStepInfo)):
    # Read the full SOIl result for time step i
    soilResults = case.properties.activeCellProperty('DYNAMIC_NATIVE', 'SOIL', i)
    
    # Generate the result by looping through both lists in order
    results = []
    for (soil, porv) in zip(soilResults, porvResults):
        results.append(soil * porv)

    # Send back result
    case.properties.setActiveCellProperty(results, 'GENERATED', 'SOILPORVSync', i)

end = time.time()
print("Time elapsed: ", end - start)

print("Transferred all results back")

view = case.views()[0].applyCellResult('GENERATED', 'SOILPORVSync')