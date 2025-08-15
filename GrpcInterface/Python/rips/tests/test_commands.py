import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import rips
import dataroot


def test_create_lgr_well(rips_instance, initialize_test):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )
    assert case.name == "TEST10K_FLT_LGR_NNC"
    assert len(case.grids()) == 2

    well_files = [dataroot.PATH + "/TEST10K_FLT_LGR_NNC/wellpath_a.dev"]
    rips_instance.project.import_well_paths(well_path_files=well_files)

    time_step = 5
    well_path_names = ["Well Path A"]
    case.create_lgr_for_completion(
        time_step,
        well_path_names,
        refinement_i=2,
        refinement_j=3,
        refinement_k=1,
        split_type="LGR_PER_WELL",
    )
    assert len(case.grids()) == 3


def test_create_multiple_fractures(rips_instance, initialize_test):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )

    well_files = [dataroot.PATH + "/TEST10K_FLT_LGR_NNC/wellpath_a.dev"]
    rips_instance.project.import_well_paths(well_path_files=well_files)

    fracture_instances = rips_instance.project.descendants(rips.WellPathFracture)
    assert len(fracture_instances) == 0

    case.create_multiple_fractures(
        template_id=0,
        well_path_names="Well Path A",
        min_dist_from_well_td=100.0,
        max_fractures_per_well=5,
        top_layer=1,
        base_layer=30,
        spacing=100.0,
        action="REPLACE_FRACTURES",
    )

    fracture_instances = rips_instance.project.descendants(rips.WellPathFracture)
    assert len(fracture_instances) > 0
