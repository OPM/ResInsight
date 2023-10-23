import sys
import os
from typing import Any, Dict, List, TypedDict
import math

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


class NameAndPath(TypedDict):
    # Typed dict container for name of property and path of property file
    name: str
    path: str


def check_corner(actual, expected):
    assert math.isclose(actual.x, expected[0], abs_tol=0.1)
    assert math.isclose(actual.y, expected[1], abs_tol=0.1)
    assert math.isclose(actual.z, expected[2], abs_tol=0.1)


def test_10k(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)
    assert len(case.grids()) == 2
    grid = case.grid(index=0)
    dimensions = grid.dimensions()
    assert dimensions.i == 90
    assert dimensions.j == 96
    assert dimensions.k == 36

    cell_centers = grid.cell_centers()
    assert len(cell_centers) == (dimensions.i * dimensions.j * dimensions.k)

    property_data_index = grid.property_data_index_from_ijk(31, 53, 21)
    assert property_data_index == 177510

    # Test a specific cell (results from ResInsight UI)
    cell_index = 168143
    assert math.isclose(3627.17, cell_centers[cell_index].x, abs_tol=0.1)
    assert math.isclose(5209.75, cell_centers[cell_index].y, abs_tol=0.1)
    assert math.isclose(4179.6, cell_centers[cell_index].z, abs_tol=0.1)

    cell_corners = grid.cell_corners()
    assert len(cell_corners) == (dimensions.i * dimensions.j * dimensions.k)

    # Expected values from ResInsight UI
    expected_corners = [
        [3565.22, 5179.02, 4177.18],
        [3655.67, 5145.34, 4176.63],
        [3690.07, 5240.69, 4180.02],
        [3599.87, 5275.16, 4179.32],
        [3564.13, 5178.61, 4179.75],
        [3654.78, 5144.79, 4179.23],
        [3688.99, 5239.88, 4182.7],
        [3598.62, 5274.48, 4181.96],
    ]
    check_corner(cell_corners[cell_index].c0, expected_corners[0])
    check_corner(cell_corners[cell_index].c1, expected_corners[1])
    check_corner(cell_corners[cell_index].c2, expected_corners[2])
    check_corner(cell_corners[cell_index].c3, expected_corners[3])
    check_corner(cell_corners[cell_index].c4, expected_corners[4])
    check_corner(cell_corners[cell_index].c5, expected_corners[5])
    check_corner(cell_corners[cell_index].c6, expected_corners[6])
    check_corner(cell_corners[cell_index].c7, expected_corners[7])


def check_reek_grid_box(case: rips.Case):
    assert len(case.grids()) == 1
    grid = case.grid(index=0)

    dimensions = grid.dimensions()
    assert dimensions.i == 21
    assert dimensions.j == 23
    assert dimensions.k == 14

    cell_centers = grid.cell_centers()
    total_size = dimensions.i * dimensions.j * dimensions.k
    assert len(cell_centers) == total_size

    poro = case.active_cell_property("INPUT_PROPERTY", "PORO", 0)
    assert len(poro) == total_size
    assert math.isclose(min(poro), 0.000928084715269506)
    assert math.isclose(max(poro), 0.351595014333725)


def test_load_roff_binary_grid(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/reek/reek_box_grid_w_props.roff"
    case = rips_instance.project.load_case(path=casePath)
    check_reek_grid_box(case)


def test_load_roff_ascii_grid(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/reek/reek_box_grid_w_props.roffasc"
    case = rips_instance.project.load_case(path=casePath)
    check_reek_grid_box(case)


def test_load_grdecl_grid(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/reek/reek_box_grid_w_props.grdecl"
    case = rips_instance.project.load_case(path=casePath)
    check_reek_grid_box(case)


def verify_load_grid_and_separate_properties(
    case: rips.Reservoir, property_name_and_paths: NameAndPath
):
    # Load case without properties
    available_properties = case.available_properties("INPUT_PROPERTY")
    for [name, _path] in property_name_and_paths.items():
        assert name not in available_properties

    grid = case.grid(index=0)
    dimensions = grid.dimensions()
    total_size = dimensions.i * dimensions.j * dimensions.k

    # Import properties to case
    case.import_properties(file_names=list(property_name_and_paths.values()))
    available_properties = case.available_properties("INPUT_PROPERTY")
    for [name, _path] in property_name_and_paths.items():
        assert name in available_properties
        property_values = case.active_cell_property("INPUT_PROPERTY", name, 0)
        assert len(property_values) == total_size


def test_load_grdecl_grid_with_separate_properties(rips_instance, initialize_test):
    grid_file_path: str = dataroot.PATH + "/reek/reek_box_grid_w_out_props.grdecl"
    property_name_and_paths: NameAndPath = {
        "EQLNUM": dataroot.PATH + "/reek/reek_box_EQLNUM_property.grdecl",
        "PORO": dataroot.PATH + "/reek/reek_box_PORO_property.grdecl",
    }
    case = rips_instance.project.load_case(path=grid_file_path)

    verify_load_grid_and_separate_properties(case, property_name_and_paths)


def test_load_roff_grid_with_separate_properties(rips_instance, initialize_test):
    grid_file_path: str = dataroot.PATH + "/reek/reek_box_grid_w_out_props.roffasc"
    property_name_and_paths: NameAndPath = {
        "EQLNUM": dataroot.PATH + "/reek/reek_box_EQLNUM_property.roffasc",
        "PORO": dataroot.PATH + "/reek/reek_box_PORO_property.roffasc",
    }
    case = rips_instance.project.load_case(path=grid_file_path)

    verify_load_grid_and_separate_properties(case, property_name_and_paths)
