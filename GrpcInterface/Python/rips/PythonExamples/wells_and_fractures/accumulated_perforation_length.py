###################################################################################
# This example will connect to ResInsight, retrieve a list of
# simulation wells for a case and get the accumulated perforated length for all
# simulation wells per timestep. If a well is closed at a given timestep, the
# accumulated perforated length will be zero.
###################################################################################

# Import the ResInsight Processing Server Module
import rips
import sys

# Connect to ResInsight
resinsight = rips.Instance.find()
if resinsight is None:
    sys.exit("ResInsight is not running. Please start ResInsight and try again.")

# Get a list of all wells
cases = resinsight.project.cases()
if len(cases) == 0:
    sys.exit("No cases found in the project. Please open a case and try again.")

case = cases[0]
print("Using Case: " + case.name)

timesteps = case.time_steps()

# store results in a dictionary,
# use well name as the key and a list of the wells accumulated perforation
# lengths for each timestep as the value
results = {}

# loop over all available simulation wells
sim_wells = case.simulation_wells()
for sim_well in sim_wells:
    acclengths = []
    for tidx, timestep in enumerate(timesteps):
        acclengths.append(sim_well.accumulated_perforation_length(tidx))
    results[sim_well.name] = acclengths

# Print header
header = ["Timestep"]
for tidx, ts in enumerate(timesteps):
    header.append("%d/%d/%d" % (ts.day, ts.month, ts.year))
print(";".join(header))

# Print the results
for well_name, lengths in results.items():
    line = [well_name]
    for idx, length in enumerate(lengths):
        line.append(str(length))
    print(";".join(line))
