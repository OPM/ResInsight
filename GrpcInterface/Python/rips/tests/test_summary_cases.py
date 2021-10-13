import sys
import os
import math
import contextlib
import os
import shutil
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

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
    # Summary reader type is controlled from Preferences. libecl reports 343 vectors, opm_common (ESMRY) reports 310.
    # As this configuration can be different, allow both variants
    assert len(addresses.values) == 343 or len(addresses.values) == 310

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


# This test ensures that missing unsmry file is handeled gracefully
def test_summary_no_unsmry(rips_instance, initialize_test):
    casePathRelative = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"

    # create an absolute path, as the helper functions used to create a temporary folder does not work
    # with the relative (..\..\) part of the file path
    casePath = os.path.abspath(casePathRelative)

    with tempdir() as dirpath:
        base_path = os.path.basename(casePath)
        temp_path = os.path.join(dirpath, base_path)
        shutil.copy2(casePath, temp_path)

        summary_case = rips_instance.project.import_summary_case(temp_path)
        assert summary_case is None
