import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

import dataroot

def test_10k(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert(len(case.grids()) == 2)
    well_path_files = [case_root_path + "/wellpath_a.dev", case_root_path + "/wellpath_b.dev"]
    well_paths = rips_instance.project.import_well_paths(well_path_files)
    wells = rips_instance.project.wells()
    assert(len(wells) == 2)
    assert(wells[0].name == "Well Path A")
    assert(wells[1].name == "Well Path B")
