import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import dataroot


def _open_case(rips_instance):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )
    return project.cases()[0]


def test_combined_filter_round_trip(rips_instance, initialize_test):
    case = _open_case(rips_instance)

    data_filters = case.data_filter_collection()
    assert data_filters is not None
    assert len(data_filters.filters()) == 0

    combined = data_filters.add_combined_filter(name="My Combined", combine_mode="OR")
    assert combined is not None
    assert combined.name == "My Combined"
    assert combined.combine_mode == "OR"
    assert len(data_filters.filters()) == 1


def test_combined_filter_add_property_filter(rips_instance, initialize_test):
    case = _open_case(rips_instance)
    data_filters = case.data_filter_collection()

    combined = data_filters.add_combined_filter(combine_mode="AND")

    pf = combined.add_property_filter(
        result_variable="SOIL", result_type="DYNAMIC_NATIVE"
    )
    assert pf is not None
    assert len(combined.filters()) == 1

    pf2 = combined.add_property_filter(
        result_variable="PRESSURE", result_type="DYNAMIC_NATIVE"
    )
    assert pf2 is not None
    assert len(combined.filters()) == 2

    # Bounds are populated by setToDefaultValues based on the result range; override
    # them via the scriptable fields.
    pf2.lower_bound = 150.0
    pf2.upper_bound = 250.0
    pf2.update()

    refreshed = combined.filters()[1]
    assert refreshed.lower_bound == 150.0
    assert refreshed.upper_bound == 250.0


def test_combined_filter_add_range_filter(rips_instance, initialize_test):
    case = _open_case(rips_instance)
    data_filters = case.data_filter_collection()

    combined = data_filters.add_combined_filter()

    rf = combined.add_range_filter(name="K-slice 5", start_k=5, cell_count_k=1)
    assert rf is not None
    assert "K-slice 5" in rf.name
    assert rf.start_index_k == 5
    assert rf.cell_count_k == 1

    # Defaults from setDefaultValues should populate I/J using grid extent (>= 1).
    assert rf.start_index_i >= 1
    assert rf.start_index_j >= 1
    assert rf.cell_count_i >= 1
    assert rf.cell_count_j >= 1

    # Mutate via field assignment + update.
    rf.cell_count_k = 3
    rf.update()

    refreshed = combined.filters()[0]
    assert refreshed.cell_count_k == 3


def test_combined_filter_mode_toggle(rips_instance, initialize_test):
    case = _open_case(rips_instance)
    data_filters = case.data_filter_collection()

    combined = data_filters.add_combined_filter(combine_mode="OR")
    assert combined.combine_mode == "OR"

    combined.combine_mode = "AND"
    combined.update()

    refreshed = data_filters.filters()[0]
    assert refreshed.combine_mode == "AND"
