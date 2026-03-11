import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import dataroot


def test_well_path_color(rips_instance, initialize_test):
    """Test reading and setting well path colors via Python API."""
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    rips_instance.project.load_case(path=case_path)

    well_path_files = [
        case_root_path + "/wellpath_a.dev",
        case_root_path + "/wellpath_b.dev",
    ]
    rips_instance.project.import_well_paths(well_path_files)
    wells = rips_instance.project.well_paths()
    assert len(wells) == 2

    # Read default color
    well_a = wells[0]
    well_b = wells[1]
    assert well_a.well_path_color is not None
    assert well_b.well_path_color is not None

    # Set colors using hex strings
    well_a.well_path_color = "#ff0000"
    well_a.update()

    well_b.well_path_color = "#0000ff"
    well_b.update()

    # Re-fetch and verify colors persist
    wells = rips_instance.project.well_paths()
    assert wells[0].well_path_color == "#ff0000"
    assert wells[1].well_path_color == "#0000ff"
