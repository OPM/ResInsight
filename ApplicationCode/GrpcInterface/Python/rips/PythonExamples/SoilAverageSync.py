###########################################################################################
# This example will synchronously calculate the average value for SOIL for all time steps
###########################################################################################
import rips
import itertools
import time

resinsight     = rips.Instance.find()

start          = time.time()

# Get the case with case id 0
case           = resinsight.project.case(case_id=0)

# Get a list of all time steps
time_steps      = case.time_steps()

averages = []
for i in range(0, len(time_steps)):
    # Get a list of all the results for time step i
	results = case.active_cell_property('DYNAMIC_NATIVE', 'SOIL', i)
	mysum = sum(results)
	averages.append(mysum/len(results))

end = time.time()
print("Time elapsed: ", end - start)
print(averages)