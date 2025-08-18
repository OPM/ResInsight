import rips


def test_diameter_roughness_intervals(rips_instance, initialize_test):
    """Test the new diameter roughness intervals Python GRPC interface"""

    # Create a test well
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "Test Well for Diameter Intervals"

    completions_settings = well_path.completion_settings()
    assert completions_settings is not None, "Completion settings should not be None"

    # Test our new method - create first interval
    interval1 = completions_settings.add_diameter_roughness_interval(
        start_md=100, end_md=200, diameter=0.15, roughness_factor=1e-5
    )

    # Assertions to verify the first interval
    assert interval1 is not None, "First interval should not be None"
    assert interval1.start_md == 100
    assert interval1.end_md == 200
    assert interval1.diameter == 0.15
    assert interval1.roughness_factor == 1e-5

    # Test creating a second interval with different values
    interval2 = completions_settings.add_diameter_roughness_interval(
        start_md=200, end_md=300, diameter=0.12, roughness_factor=2e-5
    )

    # Assertions to verify the second interval
    assert interval2 is not None, "Second interval should not be None"
    assert interval2.start_md == 200
    assert interval2.end_md == 300
    assert interval2.diameter == 0.12
    assert interval2.roughness_factor == 2e-5

    # Test default values
    interval3 = completions_settings.add_diameter_roughness_interval()

    assert interval3 is not None, "Third interval with defaults should not be None"
    assert interval3.start_md == 0.0
    assert interval3.end_md == 100.0
    assert interval3.diameter == 0.152
    assert interval3.roughness_factor == 1e-5
