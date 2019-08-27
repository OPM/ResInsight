###########################################################################################
# This example will synchronously calculate the average value for SOIL for all time steps
###########################################################################################
import rips
import itertools
import time

resInsight     = rips.Instance.find()

start          = time.time()
case           = resInsight.project.case(id=0)

# Get the case with case id 0
case           = resInsight.project.case(id=0)

# Get a list of all time steps
timeSteps      = case.timeSteps()

averages = []
for i in range(0, len(timeSteps)):
    # Get a list of all the results for time step i
	results = case.properties.activeCellProperty('DYNAMIC_NATIVE', 'SOIL', i)
	mysum = sum(results)
	averages.append(mysum/len(results))

end = time.time()
print("Time elapsed: ", end - start)
print(averages)