import os
import sys

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


def test_combined_filter_named_preserves_name(rips_instance, initialize_test):
    """User-supplied name is kept verbatim and does not change when child bounds change."""
    case = _open_case(rips_instance)
    data_filters = case.data_filter_collection()

    combined = data_filters.add_combined_filter(name="My Custom", combine_mode="AND")
    pf = combined.add_property_filter(
        result_variable="SOIL", result_type="DYNAMIC_NATIVE"
    )
    pf.lower_bound = 0.3
    pf.upper_bound = 0.6
    pf.update()

    refreshed = data_filters.filters()[0]
    assert refreshed.name == "My Custom"


def test_combined_filter_auto_derived_name_tracks_children(
    rips_instance, initialize_test
):
    """No name supplied: combined-filter name is auto-derived and updates when children change."""
    case = _open_case(rips_instance)
    data_filters = case.data_filter_collection()

    combined = data_filters.add_combined_filter(combine_mode="AND")

    # Empty: falls back to the default placeholder.
    refreshed_empty = data_filters.filters()[0]
    assert refreshed_empty.name == "Combined Filter"

    pf = combined.add_property_filter(
        result_variable="SOIL", result_type="DYNAMIC_NATIVE"
    )
    pf.lower_bound = 0.3
    pf.upper_bound = 0.6
    pf.update()

    refreshed = data_filters.filters()[0]
    name_at_lower_03 = refreshed.name
    assert "0.3" in name_at_lower_03
    assert "SOIL" in name_at_lower_03

    # Add a second child — auto-derive should append it with the AND separator.
    combined.add_range_filter(name="K=5..10", start_k=5, cell_count_k=6)
    refreshed = data_filters.filters()[0]
    assert " AND " in refreshed.name
    assert "K=5..10" in refreshed.name

    # Mutate the lower bound — auto-derive picks up the new value.
    pf.lower_bound = 0.4
    pf.update()
    refreshed = data_filters.filters()[0]
    assert "0.4" in refreshed.name
    assert "0.3" not in refreshed.name


def test_combined_filter_user_rename_disables_auto_derive(
    rips_instance, initialize_test
):
    """Renaming an auto-derived combined filter via Python locks the name in place."""
    case = _open_case(rips_instance)
    data_filters = case.data_filter_collection()

    combined = data_filters.add_combined_filter(combine_mode="AND")
    pf = combined.add_property_filter(
        result_variable="SOIL", result_type="DYNAMIC_NATIVE"
    )
    pf.lower_bound = 0.3
    pf.upper_bound = 0.6
    pf.update()

    # Rename via Python: that disables auto-derive.
    combined.name = "Locked Name"
    combined.update()

    pf.lower_bound = 0.4
    pf.update()

    refreshed = data_filters.filters()[0]
    assert refreshed.name == "Locked Name"
