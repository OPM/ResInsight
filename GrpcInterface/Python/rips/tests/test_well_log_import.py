import pytest
import rips

import os


@pytest.fixture
def project(rips_instance):
    """Create a test project with a well path"""
    # Create well path from coordinates
    coordinates = [
        [1000, 2000, 0],
        [1000, 2000, 1000],
        [1100, 2100, 2000],
        [1200, 2200, 3000],
    ]

    well_path = (
        rips_instance.project.well_path_collection().import_well_path_from_points(
            name="TestWell", coordinates=coordinates
        )
    )

    return rips_instance.project, well_path


def test_add_well_log_basic(project):
    """Test basic well log import functionality"""
    proj, well_path = project

    # Test data
    name = "TestLog"
    values = [10.5, 20.3, 30.8, 40.2, 15.7]

    # Import well log
    well_log = well_path.add_well_log(name=name, values=values)

    # Verify well log was created
    assert well_log is not None
    assert well_log.name == name


def test_add_well_log_empty_name(project):
    """Test that empty name raises ValueError"""
    proj, well_path = project

    values = [1.0, 2.0, 3.0]

    with pytest.raises(ValueError, match="Name cannot be empty"):
        well_path.add_well_log(name="", values=values)


def test_add_well_log_empty_values(project):
    """Test that empty values raises ValueError"""
    proj, well_path = project

    with pytest.raises(ValueError, match="Values list cannot be empty"):
        well_path.add_well_log(name="TestLog", values=[])


def test_add_well_log_invalid_values_type(project):
    """Test that non-list values raises TypeError"""
    proj, well_path = project

    with pytest.raises(TypeError, match="Values must be a list"):
        well_path.add_well_log(name="TestLog", values="not a list")


def test_add_well_log_non_numeric_values(project):
    """Test that non-numeric values raises TypeError"""
    proj, well_path = project

    values = [1.0, "invalid", 3.0]

    with pytest.raises(TypeError, match="All values must be numeric"):
        well_path.add_well_log(name="TestLog", values=values)


def test_add_well_log_mixed_numeric_types(project):
    """Test that mixed numeric types work correctly"""
    proj, well_path = project

    # Mix of int, float
    values = [1, 2.5, 3, 4.8, 5]

    well_log = well_path.add_well_log(name="MixedLog", values=values)

    assert well_log is not None
    assert well_log.name == "MixedLog"


def test_add_multiple_well_logs(project):
    """Test adding multiple well logs to same well path"""
    proj, well_path = project

    # Add first well log
    log1 = well_path.add_well_log(name="Log1", values=[1.0, 2.0, 3.0])

    # Add second well log
    log2 = well_path.add_well_log(name="Log2", values=[10.0, 20.0, 30.0])

    # Verify both logs exist
    assert log1 is not None
    assert log2 is not None
    assert log1.name == "Log1"
    assert log2.name == "Log2"

    # Both logs were created successfully (verified by assertions above)
    # Note: well_logs() method not currently exposed in Python API


def test_add_well_log_large_dataset(project):
    """Test importing large dataset"""
    proj, well_path = project

    # Generate large dataset
    values = [float(i) * 0.1 for i in range(1000)]

    well_log = well_path.add_well_log(name="LargeLog", values=values)

    assert well_log is not None
    assert well_log.name == "LargeLog"


def test_add_well_log_single_value(project):
    """Test adding well log with single value"""
    proj, well_path = project

    values = [42.0]

    well_log = well_path.add_well_log(name="SingleValue", values=values)

    assert well_log is not None
    assert well_log.name == "SingleValue"


def test_add_well_log_special_numeric_values(project):
    """Test adding well log with special numeric values"""
    proj, well_path = project

    values = [0.0, -5.5, 1e6, 1.23e-4, 99.999]

    well_log = well_path.add_well_log(name="SpecialValues", values=values)

    assert well_log is not None
    assert well_log.name == "SpecialValues"
