###################################################################################
# This example will connect to ResInsight, retrieve a list of
# simulation wells and print info
###################################################################################

# Import the ResInsight Processing Server Module
import rips

# Connect to ResInsight
resinsight  = rips.Instance.find()
if resinsight is not None:
    # Get a list of all wells
    cases = resinsight.project.cases()

    for case in cases:
        print("Case id: " + str(case.id))
        print("Case name: " + case.name)

        timesteps = case.time_steps()
        sim_wells = case.simulation_wells()
        for sim_well in sim_wells:
            print("Simulation well: " + sim_well.name)

            for (tidx, timestep) in enumerate(timesteps):
                status = sim_well.status(tidx)
                cells = sim_well.cells(tidx)
                print("timestep: " + str(tidx) + " type: " + status.well_type + " open: " + str(status.is_open) + " cells:" + str(len(cells)))
