import sys
import os
import math
import pytest
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_Launch(rips_instance, initialize_test):
    assert rips_instance is not None


def test_EmptyProject(rips_instance, initialize_test):
    cases = rips_instance.project.cases()
    assert len(cases) == 0


def test_OneCase(rips_instance, initialize_test):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )
    assert case.name == "TEST10K_FLT_LGR_NNC"
    assert case.id == 0
    cases = rips_instance.project.cases()
    assert len(cases) == 1


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
    assert case.address() != 0
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
    assert len(cases) == 1

    case.replace(new_grid_file=case_path)
    # Check that the case object has been changed
    assert case.name == "BRUGGE_0000"
    assert case.id == 0

    cases = rips_instance.project.cases()
    assert len(cases) == 1
    # Check that retrieving the case object again will yield the changed object
    case = project.case(case_id=0)
    assert case.name == "BRUGGE_0000"
    assert case.id == 0


def test_replace_grid(rips_instance, initialize_test):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    case = project.case(case_id=0)
    assert case is not None
    assert case.name == "TEST10K_FLT_LGR_NNC"

    case.replace_grid(new_grid_file=case_path)

    # The project is reloaded, so retrieve the case object again
    cases = rips_instance.project.cases()
    assert len(cases) == 1
    case = rips_instance.project.case(case_id=0)
    assert case.name == "BRUGGE_0000"
    assert case.id == 0


def test_replace_deprecated(rips_instance, initialize_test):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    case = project.case(case_id=0)
    with pytest.warns(DeprecationWarning):
        case.replace(new_grid_file=case_path)
    assert case.name == "BRUGGE_0000"


def test_loadNonExistingCase(rips_instance, initialize_test):
    case_path = "Nonsense/Nonsense/Nonsense"
    with pytest.raises(rips.RipsError):
        assert rips_instance.project.load_case(case_path)


def test_exportFlowCharacteristics(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    case = rips_instance.project.load_case(case_path)
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        print("Temporary folder: ", tmpdirname)
        file_name = tmpdirname + "/exportFlowChar.txt"
        case.export_flow_characteristics(
            time_steps=[8], producers=[], injectors=["I01"], file_name=file_name
        )
        assert os.path.exists(file_name)
        with open(file_name) as f:
            assert len(f.read()) > 0

        with pytest.raises(rips.RipsError):
            case.export_flow_characteristics(time_steps=[8])


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
        soil_results = case.selected_cell_property(
            rips.PropertyType.DYNAMIC_NATIVE, "SOIL", tidx
        )
        assert len(soil_results) == 0


def test_selected_cells_async(rips_instance, initialize_test):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )
    accumulated = []
    for chunk in case.selected_cells_async():
        accumulated.extend(chunk.cells)
    assert accumulated == case.selected_cells()


def test_selected_cell_property_async(rips_instance, initialize_test):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )
    accumulated = []
    for chunk in case.selected_cell_property_async(
        rips.PropertyType.DYNAMIC_NATIVE, "SOIL", 0
    ):
        accumulated.extend(chunk.values)
    assert accumulated == case.selected_cell_property(
        rips.PropertyType.DYNAMIC_NATIVE, "SOIL", 0
    )


def test_multiple_load_of_same_case(rips_instance, initialize_test):
    # Test related to issue https://github.com/OPM/ResInsight/issues/9332
    path_name = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.EGRID"
    case_count = 3
    for i in range(case_count):
        c = rips_instance.project.load_case(path_name)
        assert c


def test_10k_property_for_positions(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2
    time_steps = case.time_steps()
    assert len(time_steps) == 9

    # Expected values from ResInsight UI (cell ijk=[23, 44, 19])
    positions = [
        [3655.67, 5145.34, 4176.63],
        [3690.07, 5240.69, 4180.02],
        [3599.87, 5275.16, 4179.32],
        [3654.78, 5144.79, 4179.23],
        [3688.99, 5239.88, 4182.7],
        [3598.62, 5274.48, 4181.96],
    ]

    property_type = rips.PropertyType.STATIC_NATIVE
    property_name = "DX"
    porosity_model = rips.PorosityModelType.MATRIX_MODEL
    time_step = 0
    result = case.grid_property_for_positions(
        positions, property_type, property_name, time_step, porosity_model
    )
    assert result
    assert len(result) == 6

    for dx in result:
        assert dx > 96 and dx < 99


def test_10k_property_for_positions_outside_of_model(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    positions = [
        [3655.67, 5145.34, -4176.63],
        [3690.07, 52400.69, 4180.02],
    ]

    property_type = rips.PropertyType.STATIC_NATIVE
    property_name = "DX"
    porosity_model = rips.PorosityModelType.MATRIX_MODEL
    time_step = 0
    result = case.grid_property_for_positions(
        positions, property_type, property_name, time_step, porosity_model
    )

    assert result
    assert len(result) == 2
    for dx in result:
        assert math.isinf(dx)


def test_10k_property_for_positions_no_positions(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    positions = []

    property_type = rips.PropertyType.STATIC_NATIVE
    property_name = "DX"
    porosity_model = rips.PorosityModelType.MATRIX_MODEL
    time_step = 0
    with pytest.raises(rips.RipsError, match="Invalid positions specified"):
        case.grid_property_for_positions(
            positions, property_type, property_name, time_step, porosity_model
        )


def test_10k_property_for_positions_invalid_property_name(
    rips_instance, initialize_test
):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    positions = [
        [3655.67, 5145.34, 4176.63],
    ]

    property_type = rips.PropertyType.STATIC_NATIVE
    invalid_property_name = "NON_EXISTING_RESULT"
    porosity_model = rips.PorosityModelType.MATRIX_MODEL
    time_step = 0
    with pytest.raises(rips.RipsError, match="Result property not found."):
        case.grid_property_for_positions(
            positions, property_type, invalid_property_name, time_step, porosity_model
        )


def test_10k_property_for_positions_invalid_property_type(
    rips_instance, initialize_test
):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    positions = [
        [3655.67, 5145.34, 4176.63],
    ]

    property_type = "NON_EXISTING_PROPERTY_TYPE"
    invalid_property_name = "PRESSURE"
    porosity_model = rips.PorosityModelType.MATRIX_MODEL
    time_step = 0
    with pytest.raises(rips.RipsError, match="Invalid property type."):
        case.grid_property_for_positions(
            positions, property_type, invalid_property_name, time_step, porosity_model
        )


def test_10k_property_for_positions_invalid_time_step(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    time_steps = case.time_steps()
    assert len(time_steps) == 9

    positions = [
        [3655.67, 5145.34, 4176.63],
    ]

    property_type = rips.PropertyType.DYNAMIC_NATIVE
    property_name = "PRESSURE"
    porosity_model = rips.PorosityModelType.MATRIX_MODEL
    invalid_time_step = 99
    with pytest.raises(rips.RipsError, match="Invalid time step."):
        case.grid_property_for_positions(
            positions, property_type, property_name, invalid_time_step, porosity_model
        )


def test_10k_grid_property_strenum(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    from_str = case.grid_property("DYNAMIC_NATIVE", "SOIL", 3)
    from_enum = case.grid_property(
        rips.PropertyType.DYNAMIC_NATIVE,
        "SOIL",
        3,
        porosity_model=rips.PorosityModelType.MATRIX_MODEL,
    )
    assert from_str == from_enum


def test_10k_result_alias(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    result_permx = case.grid_property(rips.PropertyType.STATIC_NATIVE, "PERMX", 0)
    result_soil = case.grid_property(rips.PropertyType.DYNAMIC_NATIVE, "SOIL", 3)

    case.add_result_alias("PERMX", "PERMW")
    case.add_result_alias("SOIL", "SPOIL")

    result_permw = case.grid_property(rips.PropertyType.STATIC_NATIVE, "PERMW", 0)
    assert len(result_permw) == len(result_permx)

    for i in range(len(result_permx)):
        assert result_permw[i] == result_permx[i]

    result_spoil = case.grid_property(rips.PropertyType.DYNAMIC_NATIVE, "SPOIL", 3)

    assert len(result_spoil) == len(result_soil)

    for i in range(len(result_spoil)):
        assert result_spoil[i] == result_soil[i]

    case.clear_result_aliases()

    with pytest.raises(rips.RipsError, match="No such result"):
        case.grid_property(rips.PropertyType.DYNAMIC_NATIVE, "SPOIL", 3)


def test_create_saturation_pressure_plots_requires_data(rips_instance, initialize_test):
    # None of the test models carry EQUIL/PBUB/PDEW data, so the method must report a clear error
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(case_path)
    with pytest.raises(rips.RipsError):
        case.create_saturation_pressure_plots()


def test_create_well_bore_stability_plot_is_geomech_only(
    rips_instance, initialize_test
):
    # No GeoMech test data is available; verify the method is exposed on the right class
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(case_path)
    assert not hasattr(case, "create_well_bore_stability_plot")
    assert hasattr(rips.GeoMechCase, "create_well_bore_stability_plot")


def _write_small_vtk_case(folder):
    # Two hexahedron cells sharing a face, with two time steps and one cell property
    points = [(x, y, z) for x in range(3) for y in range(2) for z in range(2)]
    point_index = {p: i for i, p in enumerate(points)}

    def hex_connectivity(x0):
        corners = [
            (x0, 0, 0),
            (x0 + 1, 0, 0),
            (x0 + 1, 1, 0),
            (x0, 1, 0),
            (x0, 0, 1),
            (x0 + 1, 0, 1),
            (x0 + 1, 1, 1),
            (x0, 1, 1),
        ]
        return [point_index[c] for c in corners]

    connectivity = hex_connectivity(0) + hex_connectivity(1)
    coordinates = "\n".join(f"{x} {y} {z}" for x, y, z in points)
    displacements = "\n".join("0 0 0" for _ in points)
    offsets = " ".join(str(8 * (i + 1)) for i in range(2))
    types = " ".join("12" for _ in range(2))

    property_values = [[0.1, 0.2], [0.3, 0.4]]
    for step, values in enumerate(property_values):
        vtu = f"""<?xml version="1.0"?>
<VTKFile type="UnstructuredGrid" version="0.1" byte_order="LittleEndian">
  <UnstructuredGrid>
    <Piece NumberOfPoints="{len(points)}" NumberOfCells="2">
      <Points>
        <DataArray type="Float64" Name="Coordinates" NumberOfComponents="3" format="ascii">
{coordinates}
        </DataArray>
      </Points>
      <Cells>
        <DataArray type="Int32" Name="connectivity" format="ascii">{" ".join(map(str, connectivity))}</DataArray>
        <DataArray type="Int32" Name="offsets" format="ascii">{offsets}</DataArray>
        <DataArray type="UInt8" Name="types" format="ascii">{types}</DataArray>
      </Cells>
      <PointData>
        <DataArray type="Float32" Name="disp" NumberOfComponents="3" format="ascii">
{displacements}
        </DataArray>
      </PointData>
      <CellData>
        <DataArray type="Float32" Name="porosity" format="ascii">{" ".join(map(str, values))}</DataArray>
      </CellData>
    </Piece>
  </UnstructuredGrid>
</VTKFile>
"""
        with open(os.path.join(folder, f"step_{step}.vtu"), "w") as f:
            f.write(vtu)

    pvd = """<?xml version="1.0"?>
<VTKFile type="Collection" version="0.1">
  <Collection>
    <DataSet timestep="0" file="step_0.vtu"/>
    <DataSet timestep="1" file="step_1.vtu"/>
  </Collection>
</VTKFile>
"""
    pvd_path = os.path.join(folder, "small.pvd")
    with open(pvd_path, "w") as f:
        f.write(pvd)
    return pvd_path, property_values


def test_geomech_case_results(rips_instance, initialize_test, tmp_path):
    pvd_path, property_values = _write_small_vtk_case(str(tmp_path))
    case = rips_instance.project.load_case(pvd_path)
    assert isinstance(case, rips.GeoMechCase)

    assert len(case.time_step_strings()) == 2

    positions = rips.GeoMechResultPosition
    names = case.result_names(positions.ELEMENT)
    assert "porosity" in names

    for time_step, expected in enumerate(property_values):
        values = case.result_values(
            "porosity", time_step=time_step, result_position=positions.ELEMENT
        )
        assert values == pytest.approx(expected)

    # The last frame is the default, and is the only frame in this case
    assert case.result_values(
        "porosity", time_step=1, result_position=positions.ELEMENT, frame_index=0
    ) == pytest.approx(property_values[1])

    with pytest.raises(rips.RipsError):
        case.result_values("porosity", time_step=2, result_position=positions.ELEMENT)
    with pytest.raises(rips.RipsError):
        case.result_values(
            "porosity", time_step=0, result_position=positions.ELEMENT, frame_index=1
        )
