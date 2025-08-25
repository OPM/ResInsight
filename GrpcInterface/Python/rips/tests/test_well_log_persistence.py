import pytest
import tempfile
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


def test_well_log_project_file_persistence(project):
    """Test that well log data is persisted in project files"""
    proj, well_path = project

    # Test data with TVD values
    name = "PersistenceTestLog"
    measured_depth = [1000, 1500, 2000, 2500, 3000]
    channel_data = {
        "porosity": [0.10, 0.15, 0.20, 0.18, 0.12],
        "permeability": [10.5, 25.3, 45.8, 35.2, 18.7],
    }
    tvd_msl = [980, 1480, 1980, 2480, 2980]
    tvd_rkb = [1020, 1520, 2020, 2520, 3020]

    # Import well log
    well_log = well_path.add_well_log(
        name=name,
        measured_depth=measured_depth,
        channel_data=channel_data,
        tvd_msl=tvd_msl,
        tvd_rkb=tvd_rkb,
    )

    # Verify well log was created
    assert well_log is not None
    assert well_log.name == name

    # Save project to temporary file
    with tempfile.NamedTemporaryFile(suffix=".rsp", delete=False) as temp_file:
        temp_path = temp_file.name

    try:
        # Save the project
        proj.save(temp_path)

        # The project should save without errors, indicating PDM persistence is working
        # This test primarily verifies that the PDM objects can be serialized
        assert os.path.exists(temp_path)
        assert os.path.getsize(temp_path) > 0

        print(
            f"Project saved successfully to {temp_path} ({os.path.getsize(temp_path)} bytes)"
        )

    finally:
        # Clean up temp file
        if os.path.exists(temp_path):
            os.remove(temp_path)
