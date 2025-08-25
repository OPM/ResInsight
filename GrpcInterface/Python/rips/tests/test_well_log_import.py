import pytest


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
    measured_depth = [1000, 1500, 2000, 2500, 3000]
    channel_data = {
        "porosity": [0.10, 0.15, 0.20, 0.18, 0.12],
        "permeability": [10.5, 25.3, 45.8, 35.2, 18.7],
    }

    # Import well log
    well_log = well_path.add_well_log(
        name=name, measured_depth=measured_depth, channel_data=channel_data
    )

    # Verify well log was created
    assert well_log is not None
    assert well_log.name == name


def test_add_well_log_empty_name(project):
    """Test that empty name raises ValueError"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, 0.2, 0.3]}

    with pytest.raises(ValueError, match="Name cannot be empty"):
        well_path.add_well_log(
            name="", measured_depth=measured_depth, channel_data=channel_data
        )


def test_add_well_log_empty_measured_depth(project):
    """Test that empty measured depth raises ValueError"""
    proj, well_path = project

    channel_data = {"porosity": [0.1, 0.2, 0.3]}

    with pytest.raises(ValueError, match="Measured depth list cannot be empty"):
        well_path.add_well_log(
            name="TestLog", measured_depth=[], channel_data=channel_data
        )


def test_add_well_log_empty_channel_data(project):
    """Test that empty channel data raises ValueError"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000]

    with pytest.raises(ValueError, match="Channel data dictionary cannot be empty"):
        well_path.add_well_log(
            name="TestLog", measured_depth=measured_depth, channel_data={}
        )


def test_add_well_log_invalid_measured_depth_type(project):
    """Test that non-list measured depth raises TypeError"""
    proj, well_path = project

    channel_data = {"porosity": [0.1, 0.2, 0.3]}

    with pytest.raises(TypeError, match="Measured depth must be a list"):
        well_path.add_well_log(
            name="TestLog", measured_depth="not a list", channel_data=channel_data
        )


def test_add_well_log_invalid_channel_data_type(project):
    """Test that non-dict channel data raises TypeError"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000]

    with pytest.raises(TypeError, match="Channel data must be a dictionary"):
        well_path.add_well_log(
            name="TestLog", measured_depth=measured_depth, channel_data="not a dict"
        )


def test_add_well_log_non_numeric_measured_depth(project):
    """Test that non-numeric measured depth values raise TypeError"""
    proj, well_path = project

    measured_depth = [1000, "invalid", 2000]
    channel_data = {"porosity": [0.1, 0.2, 0.3]}

    with pytest.raises(TypeError, match="All measured depth values must be numeric"):
        well_path.add_well_log(
            name="TestLog", measured_depth=measured_depth, channel_data=channel_data
        )


def test_add_well_log_non_numeric_channel_values(project):
    """Test that non-numeric channel values raise TypeError"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, "invalid", 0.3]}

    with pytest.raises(
        TypeError, match="All values in channel 'porosity' must be numeric"
    ):
        well_path.add_well_log(
            name="TestLog", measured_depth=measured_depth, channel_data=channel_data
        )


def test_add_well_log_mismatched_lengths(project):
    """Test that mismatched lengths raise ValueError"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, 0.2]}  # Only 2 values

    with pytest.raises(
        ValueError,
        match="Channel 'porosity' has 2 values but measured depth has 3 values",
    ):
        well_path.add_well_log(
            name="TestLog", measured_depth=measured_depth, channel_data=channel_data
        )


def test_add_well_log_empty_channel_name(project):
    """Test that empty channel name raises ValueError"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000]
    channel_data = {"": [0.1, 0.2, 0.3]}  # Empty channel name

    with pytest.raises(ValueError, match="Channel name cannot be empty"):
        well_path.add_well_log(
            name="TestLog", measured_depth=measured_depth, channel_data=channel_data
        )


def test_add_well_log_mixed_numeric_types(project):
    """Test that mixed numeric types work correctly"""
    proj, well_path = project

    # Mix of int, float in both measured depth and channel data
    measured_depth = [1000, 1500.5, 2000, 2500.25, 3000]
    channel_data = {
        "porosity": [0.1, 0.15, 0.2, 0.18, 0.12],
        "gamma_ray": [75, 80.5, 85, 82.3, 78],
    }

    well_log = well_path.add_well_log(
        name="MixedLog", measured_depth=measured_depth, channel_data=channel_data
    )

    assert well_log is not None
    assert well_log.name == "MixedLog"


def test_add_well_log_multiple_channels(project):
    """Test adding well log with multiple channels"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000, 2500, 3000]
    channel_data = {
        "porosity": [0.10, 0.15, 0.20, 0.18, 0.12],
        "permeability": [10.5, 25.3, 45.8, 35.2, 18.7],
        "gamma_ray": [75.2, 82.1, 88.5, 85.3, 79.8],
        "resistivity": [2.5, 3.2, 1.8, 2.1, 2.8],
    }

    well_log = well_path.add_well_log(
        name="MultiChannelLog", measured_depth=measured_depth, channel_data=channel_data
    )

    assert well_log is not None
    assert well_log.name == "MultiChannelLog"


def test_add_multiple_well_logs(project):
    """Test adding multiple well logs to same well path"""
    proj, well_path = project

    # Add first well log
    measured_depth1 = [1000, 1500, 2000]
    channel_data1 = {"porosity": [0.1, 0.2, 0.3]}
    log1 = well_path.add_well_log(
        name="Log1", measured_depth=measured_depth1, channel_data=channel_data1
    )

    # Add second well log
    measured_depth2 = [1200, 1700, 2200, 2700]
    channel_data2 = {"gamma_ray": [75, 80, 85, 82]}
    log2 = well_path.add_well_log(
        name="Log2", measured_depth=measured_depth2, channel_data=channel_data2
    )

    # Verify both logs exist
    assert log1 is not None
    assert log2 is not None
    assert log1.name == "Log1"
    assert log2.name == "Log2"


def test_add_well_log_large_dataset(project):
    """Test importing large dataset"""
    proj, well_path = project

    # Generate large dataset
    measured_depth = [float(i * 10) for i in range(1000)]  # 0, 10, 20, ..., 9990
    channel_data = {
        "porosity": [0.1 + (i % 100) * 0.001 for i in range(1000)],
        "gamma_ray": [70.0 + (i % 50) * 0.5 for i in range(1000)],
    }

    well_log = well_path.add_well_log(
        name="LargeLog", measured_depth=measured_depth, channel_data=channel_data
    )

    assert well_log is not None
    assert well_log.name == "LargeLog"


def test_add_well_log_single_value(project):
    """Test adding well log with single value"""
    proj, well_path = project

    measured_depth = [1000.0]
    channel_data = {"porosity": [0.15]}

    well_log = well_path.add_well_log(
        name="SingleValue", measured_depth=measured_depth, channel_data=channel_data
    )

    assert well_log is not None
    assert well_log.name == "SingleValue"


def test_add_well_log_special_numeric_values(project):
    """Test adding well log with special numeric values"""
    proj, well_path = project

    measured_depth = [0.0, 1000.5, 2000.0, 3000.25, 4000.0]
    channel_data = {
        "special_values": [0.0, -5.5, 1e6, 1.23e-4, 99.999],
        "porosity": [0.0, 0.15, 0.25, 0.18, 0.0],
    }

    well_log = well_path.add_well_log(
        name="SpecialValues", measured_depth=measured_depth, channel_data=channel_data
    )

    assert well_log is not None
    assert well_log.name == "SpecialValues"


def test_add_well_log_with_tvd_msl(project):
    """Test adding well log with TVD MSL values"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000, 2500, 3000]
    channel_data = {"porosity": [0.10, 0.15, 0.20, 0.18, 0.12]}
    tvd_msl = [980, 1480, 1980, 2480, 2980]  # TVD MSL values

    well_log = well_path.add_well_log(
        name="WithTvdMsl",
        measured_depth=measured_depth,
        channel_data=channel_data,
        tvd_msl=tvd_msl,
    )

    assert well_log is not None
    assert well_log.name == "WithTvdMsl"


def test_add_well_log_with_tvd_rkb(project):
    """Test adding well log with TVD RKB values"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000, 2500, 3000]
    channel_data = {"porosity": [0.10, 0.15, 0.20, 0.18, 0.12]}
    tvd_rkb = [1020, 1520, 2020, 2520, 3020]  # TVD RKB values

    well_log = well_path.add_well_log(
        name="WithTvdRkb",
        measured_depth=measured_depth,
        channel_data=channel_data,
        tvd_rkb=tvd_rkb,
    )

    assert well_log is not None
    assert well_log.name == "WithTvdRkb"


def test_add_well_log_with_both_tvd(project):
    """Test adding well log with both TVD MSL and TVD RKB values"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000, 2500, 3000]
    channel_data = {"porosity": [0.10, 0.15, 0.20, 0.18, 0.12]}
    tvd_msl = [980, 1480, 1980, 2480, 2980]  # TVD MSL values
    tvd_rkb = [1020, 1520, 2020, 2520, 3020]  # TVD RKB values

    well_log = well_path.add_well_log(
        name="WithBothTvd",
        measured_depth=measured_depth,
        channel_data=channel_data,
        tvd_msl=tvd_msl,
        tvd_rkb=tvd_rkb,
    )

    assert well_log is not None
    assert well_log.name == "WithBothTvd"


def test_add_well_log_tvd_msl_length_mismatch(project):
    """Test that TVD MSL length mismatch raises ValueError"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, 0.2, 0.3]}
    tvd_msl = [980, 1480]  # Only 2 values for 3 measured depths

    with pytest.raises(
        ValueError, match="tvd_msl has 2 values but measured depth has 3 values"
    ):
        well_path.add_well_log(
            name="TestLog",
            measured_depth=measured_depth,
            channel_data=channel_data,
            tvd_msl=tvd_msl,
        )


def test_add_well_log_tvd_rkb_length_mismatch(project):
    """Test that TVD RKB length mismatch raises ValueError"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, 0.2, 0.3]}
    tvd_rkb = [1020, 1520, 2020, 2520]  # 4 values for 3 measured depths

    with pytest.raises(
        ValueError, match="tvd_rkb has 4 values but measured depth has 3 values"
    ):
        well_path.add_well_log(
            name="TestLog",
            measured_depth=measured_depth,
            channel_data=channel_data,
            tvd_rkb=tvd_rkb,
        )


def test_add_well_log_tvd_msl_non_numeric(project):
    """Test that non-numeric TVD MSL values raise TypeError"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, 0.2, 0.3]}
    tvd_msl = [980, "invalid", 1980]

    with pytest.raises(TypeError, match="All tvd_msl values must be numeric"):
        well_path.add_well_log(
            name="TestLog",
            measured_depth=measured_depth,
            channel_data=channel_data,
            tvd_msl=tvd_msl,
        )


def test_add_well_log_tvd_rkb_non_numeric(project):
    """Test that non-numeric TVD RKB values raise TypeError"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, 0.2, 0.3]}
    tvd_rkb = [1020, 1520, "invalid"]

    with pytest.raises(TypeError, match="All tvd_rkb values must be numeric"):
        well_path.add_well_log(
            name="TestLog",
            measured_depth=measured_depth,
            channel_data=channel_data,
            tvd_rkb=tvd_rkb,
        )


def test_add_well_log_tvd_msl_invalid_type(project):
    """Test that non-list TVD MSL raises TypeError"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, 0.2, 0.3]}
    tvd_msl = "not a list"

    with pytest.raises(TypeError, match="tvd_msl must be a list"):
        well_path.add_well_log(
            name="TestLog",
            measured_depth=measured_depth,
            channel_data=channel_data,
            tvd_msl=tvd_msl,
        )


def test_add_well_log_tvd_rkb_invalid_type(project):
    """Test that non-list TVD RKB raises TypeError"""
    proj, well_path = project

    measured_depth = [1000, 1500, 2000]
    channel_data = {"porosity": [0.1, 0.2, 0.3]}
    tvd_rkb = "not a list"

    with pytest.raises(TypeError, match="tvd_rkb must be a list"):
        well_path.add_well_log(
            name="TestLog",
            measured_depth=measured_depth,
            channel_data=channel_data,
            tvd_rkb=tvd_rkb,
        )
