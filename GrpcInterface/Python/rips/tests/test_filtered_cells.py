import os
import sys

import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import dataroot


def _open_case(rips_instance):
    project = rips_instance.project.open(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp"
    )
    return project.cases()[0]


def test_filtered_cells_length_matches_grid_property(rips_instance, initialize_test):
    """Returned mask length matches grid_property length for the same grid."""
    case = _open_case(rips_instance)

    combined = case.data_filter_collection().add_combined_filter(combine_mode="AND")
    mask = case.filtered_cells(filter=combined, time_step=0, grid_index=0)
    permx = case.grid_property("STATIC_NATIVE", "PERMX", 0)

    assert len(mask) == len(permx)
    assert len(mask) > 0


def test_filtered_cells_empty_combined_passes_all(rips_instance, initialize_test):
    """An empty combined filter (no children) keeps every cell visible."""
    case = _open_case(rips_instance)

    combined = case.data_filter_collection().add_combined_filter(combine_mode="AND")
    mask = case.filtered_cells(filter=combined)

    assert all(v == 1 for v in mask)


def test_filtered_cells_range_filter_narrows(rips_instance, initialize_test):
    """A K-slice range filter restricts the mask to that slab."""
    case = _open_case(rips_instance)

    combined = case.data_filter_collection().add_combined_filter(combine_mode="AND")
    rf = combined.add_range_filter(name="K=5", start_k=5, cell_count_k=1)
    rf.update()

    dims = case.grid(0).dimensions()
    expected_in_slab = dims.i * dims.j

    mask = case.filtered_cells(filter=combined, grid_index=0)
    assert sum(mask) == expected_in_slab


def test_filtered_cells_property_filter_pins_ordering(rips_instance, initialize_test):
    """Property-filter mask agrees with the grid_property values cell-by-cell.

    For a PERMX in [lower, upper] filter, mask[i]==1 iff lower <= permx[i] <= upper.
    Identical ordering across the two vectors is the whole point of this API.
    """
    case = _open_case(rips_instance)

    permx = case.grid_property("STATIC_NATIVE", "PERMX", 0)

    finite = [v for v in permx if v == v]  # NaN-safe
    assert finite, "PERMX has no finite values in this case"
    lower = sorted(finite)[len(finite) // 4]
    upper = sorted(finite)[(3 * len(finite)) // 4]

    combined = case.data_filter_collection().add_combined_filter(combine_mode="AND")
    pf = combined.add_property_filter(
        result_variable="PERMX", result_type="STATIC_NATIVE"
    )
    pf.lower_bound = lower
    pf.upper_bound = upper
    pf.update()

    mask = case.filtered_cells(filter=combined, grid_index=0)
    assert len(mask) == len(permx)

    mismatches = 0
    for i, (m, v) in enumerate(zip(mask, permx)):
        in_range = lower <= v <= upper
        if bool(m) != bool(in_range):
            mismatches += 1
            if mismatches <= 3:
                print(
                    f"  mismatch at i={i}: mask={m}, permx={v}, range=[{lower},{upper}]"
                )
    assert mismatches == 0


def test_filtered_cells_null_filter_rejected(rips_instance, initialize_test):
    """Calling the internal method with a null filter is rejected by C++."""
    case = _open_case(rips_instance)

    with pytest.raises(Exception):
        case.filtered_cells_internal(
            filter=None, mask_key="ignored", time_step=0, grid_index=0
        )
