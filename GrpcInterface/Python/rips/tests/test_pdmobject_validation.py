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


def test_strict_double_validation_extra_text(rips_instance, initialize_test):
    """Verifies strict validation for double fields."""
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "test_double_strict_extra"
    well_path.update()

    fishbones_collection = well_path.completions().fishbones()

    # Successfully set a value
    fishbones_collection.main_bore_skin_factor = 50.0
    fishbones_collection.update()

    # Try a badly formatted value
    fishbones_collection.main_bore_skin_factor = "100.5test"
    with pytest.raises(Exception) as exc_info:
        fishbones_collection.update()

    assert "Extra characters found" in str(exc_info.value)
    assert "100.5test" in str(exc_info.value)

    # Should not overwrite when validation fails
    assert well_path.completions().fishbones().main_bore_skin_factor == 50.0


def test_strict_double_validation_text(rips_instance, initialize_test):
    """Verifies strict validation for double fields."""
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "test_double_strict"
    well_path.update()

    fishbones_collection = well_path.completions().fishbones()
    fishbones_collection.main_bore_skin_factor = "test"

    with pytest.raises(Exception) as exc_info:
        fishbones_collection.update()

    print("Error", exc_info.value)
    assert "unreadable" in str(exc_info.value)
    assert "test" in str(exc_info.value)


def test_strict_int_validation(rips_instance, initialize_test):
    """Verifies strict validation for int fields."""
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.name = "test_int_strict"
    well_path.update()

    completions_settings = well_path.completion_settings()

    # well_bore_fluid_pvt_table is int
    completions_settings.well_bore_fluid_pvt_table = 0.99

    with pytest.raises(Exception) as exc_info:
        completions_settings.update()

    assert "Extra characters found" in str(exc_info.value)
    assert "0.99" in str(exc_info.value)


def test_enum_validation_message(rips_instance, initialize_test):
    """Verifies enum validation error message lists valid options."""
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.update()

    completions_settings = well_path.completion_settings()

    # gas_inflow_eq is an Enum
    # Valid values: STD, R-G, P-P, GPP

    completions_settings.gas_inflow_eq = "InvalidValue"

    with pytest.raises(Exception) as exc_info:
        completions_settings.update()

    error_msg = str(exc_info.value)
    assert "Failed to set enum text value" in error_msg
    assert "Valid options are:" in error_msg
    assert "STD" in error_msg
    assert "GPP" in error_msg


def test_strict_enum_validation(rips_instance, initialize_test):
    """Verifies strict validation for enum fields (trailing garbage)."""
    well_path_coll = rips_instance.project.well_path_collection()
    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path.update()

    completions_settings = well_path.completion_settings()
    completions_settings.gas_inflow_eq = "R-G"
    completions_settings.update()

    # STD is valid. "STD.garbage" should fail.
    completions_settings.gas_inflow_eq = "STD.garbage"

    with pytest.raises(Exception) as exc_info:
        completions_settings.update()

    error_msg = str(exc_info.value)
    # Expect "Extra characters found" because "STD" is parsed successfully, leaving ".garbage"
    assert "Extra characters found" in error_msg


def test_strict_bool_validation(rips_instance, initialize_test):
    """Verifies strict validation for bool fields."""
    # Get or create a FractureTemplate to test bool validation
    fracture_templates = rips_instance.project.descendants(rips.FractureTemplate)

    # Skip test if no FractureTemplate available (need a project with templates)
    if len(fracture_templates) == 0:
        pytest.skip("No FractureTemplate available for testing")

    fracture_template = fracture_templates[0]

    # Test 1: True and False should work
    fracture_template.user_defined_perforation_length = True
    fracture_template.update()
    assert fracture_template.user_defined_perforation_length is True

    fracture_template.user_defined_perforation_length = False
    fracture_template.update()
    assert fracture_template.user_defined_perforation_length is False

    # Test 2: Integer 1 should fail
    fracture_template.user_defined_perforation_length = 1
    with pytest.raises(Exception) as exc_info:
        fracture_template.update()

    error_msg = str(exc_info.value)
    assert (
        "Extra characters found" in error_msg
        or "unreadable" in error_msg
        or "does not evaluate to either true or false" in error_msg
    )

    # Reset to valid value for next test
    fracture_template.user_defined_perforation_length = False
    fracture_template.update()

    # Test 3: Float 0.99 should fail
    fracture_template.user_defined_perforation_length = 0.99
    with pytest.raises(Exception) as exc_info:
        fracture_template.update()

    error_msg = str(exc_info.value)
    assert (
        "Extra characters found" in error_msg
        or "unreadable" in error_msg
        or "does not evaluate to either true or false" in error_msg
    )

    # Reset to valid value for next test
    fracture_template.user_defined_perforation_length = False
    fracture_template.update()

    # Test 4: String "yes" should fail
    fracture_template.user_defined_perforation_length = "yes"
    with pytest.raises(Exception) as exc_info:
        fracture_template.update()

    error_msg = str(exc_info.value)
    assert (
        "Extra characters found" in error_msg
        or "unreadable" in error_msg
        or "does not evaluate to either true or false" in error_msg
    )
