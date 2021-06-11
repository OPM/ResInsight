###########################################################################################
# This example will asynchronously calculate the average value for SOIL for all time steps
###########################################################################################

import rips
import itertools
import time

resinsight = rips.Instance.find()

start = time.time()

# Get the case with case id 0
case = resinsight.project.case(case_id=0)

# Get a list of all time steps
timeSteps = case.time_steps()

averages = []
for i in range(0, len(timeSteps)):
    # Get the results from time step i asynchronously
    # It actually returns a generator object almost immediately
    result_chunks = case.active_cell_property_async("DYNAMIC_NATIVE", "SOIL", i)
    mysum = 0.0
    count = 0
    # Loop through and append the average. each time we loop resultChunks
    # We will trigger a read of the input data, meaning the script will start
    # Calculating averages before the whole resultValue for this time step has been received
    for chunk in result_chunks:
        mysum += sum(chunk.values)
        count += len(chunk.values)

    averages.append(mysum / count)

end = time.time()
print("Time elapsed: ", end - start)
print(averages)
