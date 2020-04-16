import sys
import os
import math

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

import dataroot


def test_summary_import_and_find(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"
    summary_case = rips_instance.project.import_summary_case(casePath)
    assert(summary_case.id == 1)

    case_id = 234
    found_summary_case = rips_instance.project.summary_case(case_id)
    assert(found_summary_case is None)

    correct_case_id = 1
    found_summary_case = rips_instance.project.summary_case(correct_case_id)
    assert(found_summary_case is not None)

    rips_instance.project.close()
    correct_case_id = 1
    found_summary_case = rips_instance.project.summary_case(correct_case_id)
    assert(found_summary_case is None)


def test_summary_data(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"
    summary_case = rips_instance.project.import_summary_case(casePath)
    assert(summary_case.id == 1)

    addresses = summary_case.available_addresses()
    assert(len(addresses.values) == 343)

    summary_data = summary_case.summary_vector_values("FOPT")
    assert(len(summary_data.values) == 60)


def test_summary_resample(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/flow_diagnostics_test/SIMPLE_SUMMARY2.SMSPEC"
    summary_case = rips_instance.project.import_summary_case(casePath)
    assert(summary_case.id == 1)

    summary_data_sampled = summary_case.resample_values("FOPT", "NONE")
    assert(len(summary_data_sampled.values) == 60)
    assert(len(summary_data_sampled.time_steps) == 60)

    summary_data_sampled = summary_case.resample_values("FOPT", "DAY")
    assert(len(summary_data_sampled.values) == 721)
    assert(len(summary_data_sampled.time_steps) == 721)

    summary_data_sampled = summary_case.resample_values("FOPT", "MONTH")
    assert(len(summary_data_sampled.values) == 24)
    assert(len(summary_data_sampled.time_steps) == 24)

    summary_data_sampled = summary_case.resample_values("FOPT", "QUARTER")
    assert(len(summary_data_sampled.values) == 8)
    assert(len(summary_data_sampled.time_steps) == 8)

    summary_data_sampled = summary_case.resample_values("FOPT", "YEAR")
    assert(len(summary_data_sampled.values) == 3)
    assert(len(summary_data_sampled.time_steps) == 3)
