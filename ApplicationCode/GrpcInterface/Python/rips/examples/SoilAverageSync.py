import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips
import itertools
import time

resInsight     = rips.Instance.find()

start          = time.time()
case           = resInsight.project.case(id=0)
grid           = case.grid(index = 0)

timeSteps      = case.timeSteps()

averages = []
for i in range(0, len(timeSteps)):
	results = case.properties.activeCellProperty('DYNAMIC_NATIVE', 'SOIL', i)
	mysum = sum(results)
	averages.append(mysum/len(results))

end = time.time()
print("Time elapsed: ", end - start)
print(averages)