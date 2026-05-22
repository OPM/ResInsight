import sys
import os
import pytest
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_10kAsync(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    resultChunks = case.active_cell_property_async(
        rips.PropertyType.DYNAMIC_NATIVE, "SOIL", 1
    )
    mysum = 0.0
    count = 0
    for chunk in resultChunks:
        mysum += sum(chunk.values)
        count += len(chunk.values)
    average = mysum / count
    assert mysum == pytest.approx(621.768, abs=0.001)
    assert average != pytest.approx(0.0158893, abs=0.0000001)
    assert average == pytest.approx(0.0558893, abs=0.0000001)


def test_10kSync(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    results = case.active_cell_property(rips.PropertyType.DYNAMIC_NATIVE, "SOIL", 1)
    mysum = sum(results)
    average = mysum / len(results)
    assert mysum == pytest.approx(621.768, abs=0.001)
    assert average != pytest.approx(0.0158893, abs=0.0000001)
    assert average == pytest.approx(0.0558893, abs=0.0000001)


def test_10k_set(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    results = case.active_cell_property(rips.PropertyType.DYNAMIC_NATIVE, "SOIL", 1)
    case.set_active_cell_property(results, rips.PropertyType.GENERATED, "SOIL", 1)


def test_10k_set_out_of_bounds(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    results = case.active_cell_property(rips.PropertyType.DYNAMIC_NATIVE, "SOIL", 1)
    results.append(5.0)
    with pytest.raises(rips.RipsError):
        assert case.set_active_cell_property(
            results, rips.PropertyType.GENERATED, "SOIL", 1
        )


def test_10k_set_out_of_bounds_client(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    results = case.active_cell_property(rips.PropertyType.DYNAMIC_NATIVE, "SOIL", 1)
    case.chunk_size = len(results)
    results.append(5.0)
    with pytest.raises(IndexError):
        assert case.set_active_cell_property(
            results, rips.PropertyType.GENERATED, "SOIL", 1
        )


def createResult(poroChunks, permxChunks):
    for poroChunk, permxChunk in zip(poroChunks, permxChunks):
        resultChunk = []
        for poro, permx in zip(poroChunk.values, permxChunk.values):
            resultChunk.append(poro * permx)
        yield resultChunk


def checkResults(poroValues, permxValues, poropermxValues):
    for poro, permx, poropermx in zip(poroValues, permxValues, poropermxValues):
        recalc = poro * permx
        assert recalc == pytest.approx(poropermx, rel=1.0e-10)


def test_10k_PoroPermX(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    poroChunks = case.active_cell_property_async(
        rips.PropertyType.STATIC_NATIVE, "PORO", 0
    )
    permxChunks = case.active_cell_property_async(
        rips.PropertyType.STATIC_NATIVE, "PERMX", 0
    )

    case.set_active_cell_property_async(
        createResult(poroChunks, permxChunks),
        rips.PropertyType.GENERATED,
        "POROPERMXAS",
        0,
    )

    poro = case.active_cell_property(rips.PropertyType.STATIC_NATIVE, "PORO", 0)
    permx = case.active_cell_property(rips.PropertyType.STATIC_NATIVE, "PERMX", 0)
    poroPermX = case.active_cell_property(rips.PropertyType.GENERATED, "POROPERMXAS", 0)

    checkResults(poro, permx, poroPermX)


def test_10k_set_integer_active_cell_property(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    results = case.active_cell_property(rips.PropertyType.STATIC_NATIVE, "PORO", 0)
    integer_values = [int(v * 100) for v in results]

    # A name that does not end with "NUM" - only the data_type flag should
    # cause this to be treated as a discrete/category property.
    case.set_active_cell_property(
        integer_values,
        rips.PropertyType.GENERATED,
        "MY_DISCRETE_ACTIVE",
        0,
        data_type=rips.PropertyDataType.INTEGER,
    )

    round_trip = case.active_cell_property(
        rips.PropertyType.GENERATED, "MY_DISCRETE_ACTIVE", 0
    )
    assert len(round_trip) == len(integer_values)
    for expected, actual in zip(integer_values, round_trip):
        assert expected == int(actual)

    assert (
        case.property_data_type(
            property_type=rips.PropertyType.GENERATED,
            property_name="MY_DISCRETE_ACTIVE",
        )
        == rips.PropertyDataType.INTEGER
    )
    assert (
        case.property_data_type(
            property_type=rips.PropertyType.STATIC_NATIVE,
            property_name="PORO",
        )
        == rips.PropertyDataType.FLOAT
    )


def test_10k_set_integer_grid_property(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    grid = case.grid()
    grid_cell_count = grid.cell_count()
    integer_values = [i % 4 for i in range(grid_cell_count)]

    case.set_grid_property(
        integer_values,
        rips.PropertyType.GENERATED,
        "MY_DISCRETE_GRID",
        0,
        data_type=rips.PropertyDataType.INTEGER,
    )

    round_trip = case.grid_property(rips.PropertyType.GENERATED, "MY_DISCRETE_GRID", 0)
    assert len(round_trip) == len(integer_values)
    for expected, actual in zip(integer_values, round_trip):
        assert expected == int(actual)

    assert (
        case.property_data_type(
            property_type=rips.PropertyType.GENERATED,
            property_name="MY_DISCRETE_GRID",
        )
        == rips.PropertyDataType.INTEGER
    )


def test_10k_property_strings(rips_instance, initialize_test):
    # Backward-compat coverage: the property/porosity/data-type arguments accept
    # plain strings as an alternative to the typed StrEnum classes. This test
    # exercises only the string form end-to-end so a future refactor that
    # accidentally narrows the accepted types would be caught.
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    available = case.available_properties("STATIC_NATIVE", "MATRIX_MODEL")
    assert "PORO" in available
    assert "PERMX" in available

    poro = case.active_cell_property("STATIC_NATIVE", "PORO", 0)
    assert len(poro) > 0

    grid_poro = case.grid_property("STATIC_NATIVE", "PORO", 0)
    assert len(grid_poro) > 0

    integer_values = [int(v * 100) for v in poro]
    case.set_active_cell_property(
        integer_values, "GENERATED", "MY_STRING_DISCRETE", 0, data_type="INTEGER"
    )
    round_trip = case.active_cell_property("GENERATED", "MY_STRING_DISCRETE", 0)
    assert len(round_trip) == len(integer_values)


def test_10k_grid_property_async(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    chunks = case.grid_property_async(rips.PropertyType.STATIC_NATIVE, "PORO", 0)
    accumulated = []
    chunk_count = 0
    for chunk in chunks:
        accumulated.extend(chunk.values)
        chunk_count += 1
    assert chunk_count > 0

    sync_values = case.grid_property(rips.PropertyType.STATIC_NATIVE, "PORO", 0)
    assert len(accumulated) == len(sync_values)
    for async_value, sync_value in zip(accumulated, sync_values):
        assert async_value == pytest.approx(sync_value)


def test_10k_available_properties_categories(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    dynamic = case.available_properties(rips.PropertyType.DYNAMIC_NATIVE)
    assert "SOIL" in dynamic
    assert "PRESSURE" in dynamic

    cell_count_info = case.cell_count()
    zeros = [0.0] * cell_count_info.active_cell_count
    case.set_active_cell_property(
        zeros, rips.PropertyType.GENERATED, "AVAILABLE_PROBE", 0
    )

    generated = case.available_properties(rips.PropertyType.GENERATED)
    assert "AVAILABLE_PROBE" in generated


def test_exportPropertyInView(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(case_path)
    case.create_view()
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        print("Temporary folder: ", tmpdirname)
        rips_instance.set_export_folder(export_type="PROPERTIES", path=tmpdirname)
        case = rips_instance.project.cases()[0]
        view = case.views()[0]
        view.export_property()
        expected_file_name = case.name + "-" + str("3D_View") + "-" + "T0" + "-SOIL"
        full_path = tmpdirname + "/" + expected_file_name
        assert os.path.exists(full_path)
