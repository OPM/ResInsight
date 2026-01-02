import sys
import os
import contextlib
import shutil
import tempfile
import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import dataroot


def test_summary_import_and_find(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"
    summary_case = rips_instance.project.import_summary_case(casePath)
    assert summary_case.id == 1

    case_id = 234
    found_summary_case = rips_instance.project.summary_case(case_id)
    assert found_summary_case is None

    correct_case_id = 1
    found_summary_case = rips_instance.project.summary_case(correct_case_id)
    assert found_summary_case is not None

    rips_instance.project.close()
    correct_case_id = 1
    found_summary_case = rips_instance.project.summary_case(correct_case_id)
    assert found_summary_case is None


def test_summary_data(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"
    summary_case = rips_instance.project.import_summary_case(casePath)
    assert summary_case.id == 1

    addresses = summary_case.available_addresses()
    # Summary reader type is controlled from Preferences. resdata reports 343 vectors, opm_common (ESMRY) reports 339.
    # As this configuration can be different, allow both variants
    assert len(addresses.values) == 335 or len(addresses.values) == 339

    summary_data = summary_case.summary_vector_values("FOPT")
    assert len(summary_data.values) == 60


def test_summary_resample(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"
    summary_case = rips_instance.project.import_summary_case(casePath)
    assert summary_case.id == 1

    summary_data_sampled = summary_case.resample_values("FOPT", "NONE")
    assert len(summary_data_sampled.values) == 60
    assert len(summary_data_sampled.time_steps) == 60

    summary_data_sampled = summary_case.resample_values("FOPT", "DAY")
    assert len(summary_data_sampled.values) == 721
    assert len(summary_data_sampled.time_steps) == 721

    summary_data_sampled = summary_case.resample_values("FOPT", "MONTH")
    assert len(summary_data_sampled.values) == 24
    assert len(summary_data_sampled.time_steps) == 24

    summary_data_sampled = summary_case.resample_values("FOPT", "QUARTER")
    assert len(summary_data_sampled.values) == 8
    assert len(summary_data_sampled.time_steps) == 8

    summary_data_sampled = summary_case.resample_values("FOPT", "YEAR")
    assert len(summary_data_sampled.values) == 3
    assert len(summary_data_sampled.time_steps) == 3


@contextlib.contextmanager
def cd(newdir, cleanup=lambda: True):
    prevdir = os.getcwd()
    os.chdir(os.path.expanduser(newdir))
    try:
        yield
    finally:
        os.chdir(prevdir)
        cleanup()


@contextlib.contextmanager
def tempdir():
    dirpath = tempfile.mkdtemp()

    def cleanup():
        shutil.rmtree(dirpath)

    with cd(dirpath, cleanup):
        yield dirpath


def test_summary_set_values(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"
    summary_case = rips_instance.project.import_summary_case(casePath)
    assert summary_case.id == 1

    addresses = summary_case.available_addresses()
    original_keyword_count = len(addresses.values)

    summary_data = summary_case.summary_vector_values("FOPT")
    assert len(summary_data.values) == 60

    summary_case.set_summary_values("FOPT_1", "", summary_data.values)
    generated_summary_data = summary_case.summary_vector_values("FOPT_1")
    assert len(generated_summary_data.values) == 60

    addresses = summary_case.available_addresses()
    current_keyword_count = len(addresses.values)
    assert current_keyword_count == original_keyword_count + 1

    # Using existing keyword will overwrite existing data
    summary_case.set_summary_values("FOPT_1", "", summary_data.values)
    addresses = summary_case.available_addresses()
    current_keyword_count = len(addresses.values)
    assert current_keyword_count == original_keyword_count + 1

    # invalid value count should now raise an error (no longer fails silently)
    with pytest.raises(Exception) as exc_info:
        summary_case.set_summary_values("FOPT_2", "", [])
    # Verify error message is informative
    assert "size" in str(exc_info.value).lower() or "Expected" in str(exc_info.value)

    # Verify addresses are unchanged after failed operation
    addresses = summary_case.available_addresses()
    current_keyword_count = len(addresses.values)
    assert current_keyword_count == original_keyword_count + 1


def test_summary_set_values_with_long_keyword(rips_instance, initialize_test):
    """Test that setting summary values with a keyword exceeding 8 characters fails with an informative error."""
    casePath = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"
    summary_case = rips_instance.project.import_summary_case(casePath)
    assert summary_case.id == 1

    summary_data = summary_case.summary_vector_values("FOPT")
    assert len(summary_data.values) == 60

    # This keyword exceeds the 8-character SMSPEC limit
    long_keyword = "ROPR_FROM_1_TO_23"

    # Currently this fails silently - after fix, it should raise an exception with a clear error message
    with pytest.raises(Exception) as exc_info:
        summary_case.set_summary_values(long_keyword, "", summary_data.values)

    # Check that the error message is informative
    error_message = str(exc_info.value)
    assert (
        "8" in error_message
        or "character" in error_message
        or "length" in error_message
    ), f"Error message should mention the 8-character limit, but got: {error_message}"


def test_summary_set_values_with_wrong_size(rips_instance, initialize_test):
    """Test that setting summary values with wrong vector size fails with an informative error."""
    casePath = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"
    summary_case = rips_instance.project.import_summary_case(casePath)
    assert summary_case.id == 1

    summary_data = summary_case.summary_vector_values("FOPT")
    expected_size = len(summary_data.values)
    assert expected_size == 60

    # Try to set values with wrong size
    wrong_size_values = [1.0, 2.0, 3.0]  # Only 3 values instead of 60

    # Currently this fails silently - after fix, it should raise an exception with a clear error message
    with pytest.raises(Exception) as exc_info:
        summary_case.set_summary_values("TEST_KW", "", wrong_size_values)

    # Check that the error message is informative
    error_message = str(exc_info.value)
    assert "size" in error_message.lower() or "length" in error_message.lower(), (
        f"Error message should mention size/length mismatch, but got: {error_message}"
    )
    assert (
        str(expected_size) in error_message
        or str(len(wrong_size_values)) in error_message
    ), (
        f"Error message should mention expected ({expected_size}) or received ({len(wrong_size_values)}) size, but got: {error_message}"
    )
