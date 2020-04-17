import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

import dataroot


def test_10k(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert(len(case.grids()) == 2)
    cell_count_info = case.cell_count()

    sim_wells = case.simulation_wells()
    assert(len(sim_wells) == 3)

    assert(sim_wells[0].name == "GI1")
    assert(sim_wells[1].name == "GP1")
    assert(sim_wells[2].name == "GP2")

    timesteps = case.time_steps()

    # On time step 0 all simulation wells are undefined
    for sim_well in sim_wells:
        status = sim_well.status(0)
        assert(status.well_type == "NotDefined")

    # On time step 3 all wells are producing
    for sim_well in sim_wells:
        status = sim_well.status(3)
        assert(status.well_type == "Producer")

    # On time step 0 all simulation wells have no cells
    for sim_well in sim_wells:
        cells = sim_well.cells(0)
        assert(len(cells) == 0)

    # On the other time steps there should be cells
    expected_cell_count = {}
    expected_cell_count["GP1"] = 105
    expected_cell_count["GI1"] = 38
    expected_cell_count["GP2"] = 18
    for sim_well in sim_wells:
        for (tidx, timestep) in enumerate(timesteps):
            if (tidx > 0):
                cells = sim_well.cells(tidx)
                print("well: " + sim_well.name + " timestep: " +
                      str(tidx) + " cells:" + str(len(cells)))
                assert(len(cells) == expected_cell_count[sim_well.name])
