###################################################################################
# This example will connect to ResInsight, retrieve a list of
# simulation wells for a caseand get the accumulated perforated length for all
# simulation wells per timestep
###################################################################################

# Import the ResInsight Processing Server Module
import rips

# Connect to ResInsight
resinsight = rips.Instance.find()
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

            for tidx, timestep in enumerate(timesteps):
                print("  Timestep: " + str(timestep))
                acc_perforated_length = sim_well.accumulated_perforation_length(tidx)
                print(
                    "  Accumulated perforation length : " + str(acc_perforated_length)
                )
