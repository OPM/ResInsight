import pytest

import rips


def test_custom_segment_intervals(rips_instance, initialize_test):
    """Test the custom segment intervals Python GRPC interface"""

    # Create a test well
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "Test Well for Custom Segments"

    completions_settings = well_path.completion_settings()
    assert completions_settings is not None, "Completion settings should not be None"

    # Test creating first interval
    interval1 = completions_settings.add_custom_segment_interval(
        start_md=100, end_md=200
    )

    # Assertions to verify the first interval
    assert interval1 is not None, "First interval should not be None"
    assert interval1.start_md == 100
    assert interval1.end_md == 200

    # Test creating a second interval with different values
    interval2 = completions_settings.add_custom_segment_interval(
        start_md=250, end_md=350
    )

    # Assertions to verify the second interval
    assert interval2 is not None, "Second interval should not be None"
    assert interval2.start_md == 250
    assert interval2.end_md == 350

    # Test default values
    interval3 = completions_settings.add_custom_segment_interval()

    assert interval3 is not None, "Third interval with defaults should not be None"
    assert interval3.start_md == 0.0
    assert interval3.end_md == 100.0


def test_custom_segment_interval_properties(rips_instance, initialize_test):
    """Test modifying custom segment interval properties"""

    # Create a test well
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "Test Well for Property Modification"

    completions_settings = well_path.completion_settings()

    # Create an interval
    interval1 = completions_settings.add_custom_segment_interval(
        start_md=100, end_md=200
    )

    # Test modifying interval properties
    interval1.start_md = 150
    assert interval1.start_md == 150

    interval1.end_md = 250
    assert interval1.end_md == 250


def test_custom_segment_interval_invalid_range(rips_instance, initialize_test):
    """Test creating interval with invalid range (start_md > end_md) fails"""

    # Create a test well
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "Test Well for Invalid Range"

    completions_settings = well_path.completion_settings()

    # Attempt to create an interval with start_md > end_md. Should fail.
    with pytest.raises(rips.RipsError, match="End MD must be greater than Start MD"):
        completions_settings.add_custom_segment_interval(start_md=200, end_md=100)

    # Also test equal values (start_md == end_md), which should also fail
    with pytest.raises(rips.RipsError, match="End MD must be greater than Start MD"):
        completions_settings.add_custom_segment_interval(start_md=150, end_md=150)
