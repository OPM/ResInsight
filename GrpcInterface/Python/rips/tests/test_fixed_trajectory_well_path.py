import sys
import os
import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))


def test_import_fixed_trajectory_well_path_basic(rips_instance, initialize_test):
    """Test basic creation of a fixed trajectory well path"""
    well_path_coll = rips_instance.project.well_path_collection()

    # Define a simple vertical well path
    coordinates = [
        [1000.0, 2000.0, 0.0],  # Surface point
        [1000.0, 2000.0, -100.0],  # 100m down
        [1000.0, 2000.0, -500.0],  # 500m down
        [1000.0, 2000.0, -1000.0],  # 1000m down
    ]

    well_path = well_path_coll.import_fixed_trajectory_well_path(
        name="TestFixedWell", coordinates=coordinates
    )

    assert well_path is not None
    assert well_path.name == "TestFixedWell"


def test_import_fixed_trajectory_well_path_deviated(rips_instance, initialize_test):
    """Test creation of a deviated well path"""
    well_path_coll = rips_instance.project.well_path_collection()

    # Define a deviated well path
    coordinates = [
        [1000.0, 2000.0, 0.0],  # Surface point
        [1020.0, 2010.0, -200.0],  # Slight deviation
        [1050.0, 2030.0, -500.0],  # More deviation
        [1100.0, 2080.0, -1000.0],  # Final target
    ]

    well_path = well_path_coll.import_fixed_trajectory_well_path(
        name="TestDeviatedWell", coordinates=coordinates
    )

    assert well_path is not None
    assert well_path.name == "TestDeviatedWell"


def test_import_fixed_trajectory_well_path_validation(rips_instance, initialize_test):
    """Test input validation for fixed trajectory well path"""
    well_path_coll = rips_instance.project.well_path_collection()

    # Test empty name
    with pytest.raises(ValueError, match="Name cannot be empty"):
        well_path_coll.import_fixed_trajectory_well_path(
            name="", coordinates=[[1000, 2000, 0]]
        )

    # Test empty coordinates
    with pytest.raises(ValueError, match="Coordinates list cannot be empty"):
        well_path_coll.import_fixed_trajectory_well_path(
            name="TestWell", coordinates=[]
        )

    # Test invalid coordinate format - wrong number of values
    with pytest.raises(ValueError, match="must be a list/tuple of 3 values"):
        well_path_coll.import_fixed_trajectory_well_path(
            name="TestWell",
            coordinates=[[1000, 2000]],  # Missing Z coordinate
        )

    # Test invalid coordinate format - non-numeric values
    with pytest.raises(ValueError, match="All coordinate values must be numeric"):
        well_path_coll.import_fixed_trajectory_well_path(
            name="TestWell", coordinates=[[1000, "invalid", 0]]
        )


def test_import_fixed_trajectory_well_path_multiple(rips_instance, initialize_test):
    """Test creating multiple fixed trajectory well paths"""
    well_path_coll = rips_instance.project.well_path_collection()

    # Create first well path
    coordinates1 = [
        [1000.0, 2000.0, 0.0],
        [1000.0, 2000.0, -500.0],
    ]

    well_path1 = well_path_coll.import_fixed_trajectory_well_path(
        name="Well_1", coordinates=coordinates1
    )

    # Create second well path
    coordinates2 = [
        [1500.0, 2500.0, 0.0],
        [1500.0, 2500.0, -300.0],
    ]

    well_path2 = well_path_coll.import_fixed_trajectory_well_path(
        name="Well_2", coordinates=coordinates2
    )

    assert well_path1.name == "Well_1"
    assert well_path2.name == "Well_2"
    assert well_path1 != well_path2


def test_import_fixed_trajectory_well_path_single_point(rips_instance, initialize_test):
    """Test creating a well path with a single point"""
    well_path_coll = rips_instance.project.well_path_collection()

    # Single point well path
    coordinates = [
        [1000.0, 2000.0, -100.0],
    ]

    well_path = well_path_coll.import_fixed_trajectory_well_path(
        name="SinglePointWell", coordinates=coordinates
    )

    assert well_path is not None
    assert well_path.name == "SinglePointWell"


def test_import_fixed_trajectory_well_path_various_types(
    rips_instance, initialize_test
):
    """Test with various coordinate input types (list, tuple, mixed)"""
    well_path_coll = rips_instance.project.well_path_collection()

    # Mix of list and tuple coordinates
    coordinates = [
        [1000.0, 2000.0, 0.0],  # List
        (1010.0, 2010.0, -100.0),  # Tuple
        [1020, 2020, -200],  # Integers
    ]

    well_path = well_path_coll.import_fixed_trajectory_well_path(
        name="MixedTypesWell", coordinates=coordinates
    )

    assert well_path is not None
    assert well_path.name == "MixedTypesWell"


def test_import_fixed_trajectory_well_path_large_coordinates(
    rips_instance, initialize_test
):
    """Test with large coordinate values (typical UTM coordinates)"""
    well_path_coll = rips_instance.project.well_path_collection()

    # Large UTM-style coordinates
    coordinates = [
        [456789.0, 6789012.0, 50.0],  # Surface
        [456790.0, 6789013.0, -1500.0],  # Deep point
    ]

    well_path = well_path_coll.import_fixed_trajectory_well_path(
        name="UTMCoordinatesWell", coordinates=coordinates
    )

    assert well_path is not None
    assert well_path.name == "UTMCoordinatesWell"
