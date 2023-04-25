import sys
from typing import List
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_load_grdecl_grid_with_properties(rips_instance, initialize_test):
    casePath: str = dataroot.PATH + "/reek/GRID.GRDECL"
    property_file_paths: List[str] = [
        dataroot.PATH + "/reek/EQLNUM.GRDECL",
        dataroot.PATH + "/reek/PORO.GRDECL",
    ]

    # Load case and import properties
    case = rips_instance.project.load_case(path=casePath)
    case.import_properties(file_names=property_file_paths)
