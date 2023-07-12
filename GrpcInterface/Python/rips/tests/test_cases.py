import sys
import os
import math
import pytest
import grpc
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_Launch(rips_instance, initialize_test):
    assert rips_instance is not None


def test_EmptyProject(rips_instance, initialize_test):
    cases = rips_instance.project.cases()
    assert len(cases) is 0


def test_OneCase(rips_instance, initialize_test):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )
    assert case.name == "TEST10K_FLT_LGR_NNC"
    assert case.id == 0
    cases = rips_instance.project.cases()
    assert len(cases) is 1


def test_BoundingBox(rips_instance, initialize_test):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )
    assert case.name == "TEST10K_FLT_LGR_NNC"
    boundingbox = case.reservoir_boundingbox()
    assert math.isclose(3382.90, boundingbox.min_x, abs_tol=1.0e-1)
    assert math.isclose(5850.48, boundingbox.max_x, abs_tol=1.0e-1)
    assert math.isclose(4157.45, boundingbox.min_y, abs_tol=1.0e-1)
    assert math.isclose(7354.93, boundingbox.max_y, abs_tol=1.0e-1)
    assert math.isclose(-4252.61, boundingbox.min_z, abs_tol=1.0e-1)
    assert math.isclose(-4103.60, boundingbox.max_z, abs_tol=1.0e-1)

    min_depth, max_depth = case.reservoir_depth_range()
    assert math.isclose(4103.60, min_depth, abs_tol=1.0e-1)
    assert math.isclose(4252.61, max_depth, abs_tol=1.0e-1)


def test_MultipleCases(rips_instance, initialize_test):
    case_paths = []
    case_paths.append(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID")
    case_paths.append(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID")
    case_paths.append(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID")

    case_names = []
    for case_path in case_paths:
        case_name = os.path.splitext(os.path.basename(case_path))[0]
        case_names.append(case_name)
        rips_instance.project.load_case(path=case_path)

    cases = rips_instance.project.cases()
    assert len(cases) == len(case_names)
    for i, case_name in enumerate(case_names):
        assert case_name == cases[i].name


def get_cell_index_with_ijk(cell_info, i, j, k):
    for idx, cell in enumerate(cell_info):
        if cell.local_ijk.i == i and cell.local_ijk.j == j and cell.local_ijk.k == k:
            return idx
    return -1


def check_corner(actual, expected):
    assert math.isclose(actual.x, expected[0], abs_tol=0.1)
    assert math.isclose(actual.y, expected[1], abs_tol=0.1)
    assert math.isclose(actual.z, expected[2], abs_tol=0.1)


def test_10k(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2
    cell_count_info = case.cell_count()
    assert cell_count_info.active_cell_count == 11125
    assert cell_count_info.reservoir_cell_count == 316224
    time_steps = case.time_steps()
    assert len(time_steps) == 9
    days_since_start = case.days_since_start()
    assert len(days_since_start) == 9
    cell_info = case.cell_info_for_active_cells()
    assert len(cell_info) == cell_count_info.active_cell_count

    # Check an active cell (found in resinsight ui)
    cell_index = get_cell_index_with_ijk(cell_info, 23, 44, 19)
    assert cell_index != -1

    cell_centers = case.active_cell_centers()
    assert len(cell_centers) == cell_count_info.active_cell_count

    # Check the cell center for the specific cell
    assert math.isclose(3627.17, cell_centers[cell_index].x, abs_tol=0.1)
    assert math.isclose(5209.75, cell_centers[cell_index].y, abs_tol=0.1)
    assert math.isclose(4179.6, cell_centers[cell_index].z, abs_tol=0.1)

    cell_corners = case.active_cell_corners()
    assert len(cell_corners) == cell_count_info.active_cell_count
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

    # No coarsening info for this case
    coarsening_info = case.coarsening_info()
    assert len(coarsening_info) == 0


def test_PdmObject(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert case.id == 0
    assert case.address() is not 0
    assert case.__class__.__name__ == "EclipseCase"


def test_brugge_0010(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 1
    cellCountInfo = case.cell_count()
    assert cellCountInfo.active_cell_count == 43374
    assert cellCountInfo.reservoir_cell_count == 60048
    time_steps = case.time_steps()
    assert len(time_steps) == 11
    days_since_start = case.days_since_start()
    assert len(days_since_start) == 11


def test_replaceCase(rips_instance, initialize_test):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    case = project.case(case_id=0)
    assert case is not None
    assert case.name == "TEST10K_FLT_LGR_NNC"
    assert case.id == 0
    cases = rips_instance.project.cases()
    assert len(cases) is 1

    case.replace(new_grid_file=case_path)
    # Check that the case object has been changed
    assert case.name == "BRUGGE_0000"
    assert case.id == 0

    cases = rips_instance.project.cases()
    assert len(cases) is 1
    # Check that retrieving the case object again will yield the changed object
    case = project.case(case_id=0)
    assert case.name == "BRUGGE_0000"
    assert case.id == 0


def test_loadNonExistingCase(rips_instance, initialize_test):
    case_path = "Nonsense/Nonsense/Nonsense"
    with pytest.raises(grpc.RpcError):
        assert rips_instance.project.load_case(case_path)


def test_exportFlowCharacteristics(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    case = rips_instance.project.load_case(case_path)
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        print("Temporary folder: ", tmpdirname)
        file_name = tmpdirname + "/exportFlowChar.txt"
        case.export_flow_characteristics(
            time_steps=8, producers=[], injectors="I01", file_name=file_name
        )


def test_selected_cells(rips_instance, initialize_test):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )
    assert case.name == "TEST10K_FLT_LGR_NNC"
    selected_cells = case.selected_cells()
    assert len(selected_cells) == 0

    time_step_info = case.time_steps()
    for tidx, timestep in enumerate(time_step_info):
        # Try to read for SOIL the time step (will be empty since nothing is selected)
        soil_results = case.selected_cell_property("DYNAMIC_NATIVE", "SOIL", tidx)
        assert len(soil_results) == 0


def test_multiple_load_of_same_case(rips_instance, initialize_test):
    # Test related to issue https://github.com/OPM/ResInsight/issues/9332
    path_name = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.EGRID"
    case_count = 3
    for i in range(case_count):
        case = rips_instance.project.load_case(path_name)
