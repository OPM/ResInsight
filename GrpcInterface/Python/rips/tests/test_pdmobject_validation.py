"""
Test file for issue #12940: Python GRPC interface validates field values

This test verifies that validation prevents invalid values from being set.
"""

import sys
import os
import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips


def test_range_validation_minimum_enforced(rips_instance, initialize_test):
    """
    Verifies that setting value below minimum range is rejected.

    The wellBoreFluidPvtTable field has setMinValue(0) in C++, so negative values
    should be rejected with a clear error message.
    """
    well_path_coll = rips_instance.project.well_path_collection()

    # Create a new modeled well path
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "test_validation"
    well_path.update()

    # Get completion settings
    completions_settings = well_path.completion_settings()

    # Set wellBoreFluidPvtTable to a negative value (below minimum of 0)
    completions_settings.well_bore_fluid_pvt_table = -5

    # Should raise grpc._channel._InactiveRpcError with validation message
    with pytest.raises(Exception) as exc_info:
        completions_settings.update()

    error_message = str(exc_info.value)
    assert "Validation failed" in error_message
    assert (
        "WellBoreFluidPvtTable" in error_message
        or "well_bore_fluid_pvt_table" in error_message
    )
    assert "-5" in error_message or "below minimum" in error_message


def test_valid_value_should_succeed(rips_instance, initialize_test):
    """
    Control test: Valid values should always work.

    This test should pass both before and after implementing validation.
    """
    well_path_coll = rips_instance.project.well_path_collection()

    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "test_valid"
    well_path.update()

    completions_settings = well_path.completion_settings()

    # Set valid values (within range)
    completions_settings.well_bore_fluid_pvt_table = 5  # Valid: >= 0
    completions_settings.fluid_in_place_region = 10  # Valid

    # This should succeed both before and after validation implementation
    completions_settings.update()

    # Verify values were set correctly
    completions_settings_updated = well_path.completion_settings()
    assert completions_settings_updated.well_bore_fluid_pvt_table == 5
    assert completions_settings_updated.fluid_in_place_region == 10


def test_value_unchanged_after_validation_error(rips_instance, initialize_test):
    """Verifies field value doesn't change if validation fails (rollback)."""
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "test_rollback"
    well_path.update()

    completions_settings = well_path.completion_settings()

    # Set valid initial value
    completions_settings.well_bore_fluid_pvt_table = 42
    completions_settings.fluid_in_place_region = 99
    completions_settings.update()

    # Try to set invalid value
    completions_settings.well_bore_fluid_pvt_table = -5

    # Also set a different version for (should also be rolled back)
    completions_settings.fluid_in_place_region = 12

    with pytest.raises(Exception):
        completions_settings.update()

    # Value should remain at the previous valid value (42)
    completions_settings_refreshed = well_path.completion_settings()
    assert (
        completions_settings_refreshed.well_bore_fluid_pvt_table == 42
    ), "Field value should not change after failed validation (rollback should occur)"

    assert (
        completions_settings_refreshed.fluid_in_place_region == 99
    ), "Field value should not change after failed validation (rollback should occur)"
