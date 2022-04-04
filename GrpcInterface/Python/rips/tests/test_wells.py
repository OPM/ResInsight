import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_10k(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2
    well_path_files = [
        case_root_path + "/wellpath_a.dev",
        case_root_path + "/wellpath_b.dev",
    ]
    well_path_names = rips_instance.project.import_well_paths(well_path_files)
    wells = rips_instance.project.well_paths()
    assert len(wells) == 2
    assert wells[0].name == "Well Path A"
    assert wells[1].name == "Well Path B"


def test_10k_intersection(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2
    well_path_files = [
        case_root_path + "/wellpath_a.dev",
    ]

    view = case.create_view()
    view.set_time_step(1)

    well_path_names = rips_instance.project.import_well_paths(well_path_files)
    wells = rips_instance.project.well_paths()
    well_path = wells[0]

    # Add a curve intersection based on the well path
    intersection_coll = rips_instance.project.descendants(rips.IntersectionCollection)[
        0
    ]
    well_path_intersection = intersection_coll.add_new_object(rips.CurveIntersection)
    well_path_intersection.type = "CS_WELL_PATH"
    well_path_intersection.well_path = well_path
    well_path_intersection.update()

    # Three coords per triangle
    geometry = well_path_intersection.geometry()
    coord_count = len(geometry.x_coords)
    assert coord_count == 13254

    # One value per triangle
    geometry_result_values = well_path_intersection.geometry_result()
    result_count = len(geometry_result_values.values)
    assert result_count == 4418

    # Three coords per triangle, one result value per triangle
    assert (result_count * 3) == coord_count


def test_empty_well_intersection(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    view = case.create_view()
    view.set_time_step(1)

    # Add a curve intersection based on the well path
    intersection_coll = rips_instance.project.descendants(rips.IntersectionCollection)[
        0
    ]

    well_path_intersection = intersection_coll.add_new_object(rips.CurveIntersection)
    well_path_intersection.type = "CS_WELL_PATH"
    well_path_intersection.well_path = None
    well_path_intersection.update()

    # Test with empty geometry. This will also test that an empty list in CAF is converted to an empty list in Python
    # See __makelist in pdmobject.py
    geometry = well_path_intersection.geometry()
    coord_count = len(geometry.x_coords)
    assert coord_count == 0

    # One value per triangle
    geometry_result_values = well_path_intersection.geometry_result()
    result_count = len(geometry_result_values.values)
    assert result_count == 0
