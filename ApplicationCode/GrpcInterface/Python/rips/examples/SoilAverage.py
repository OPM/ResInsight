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
	resultChunks = case.properties.activeCellProperty('DYNAMIC_NATIVE', 'SOIL', i)
	mysum = 0.0
	count = 0
	for chunk in resultChunks:
		mysum += sum(chunk.values)
		count += len(chunk.values)

	averages.append(mysum/count)

end = time.time()
print("Time elapsed: ", end - start)
print(averages)