import math
import os
import sys

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import dataroot
import rips


GRID_PATH = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"


def _get_view(case):
    views = case.views()
    if views:
        return views[0]
    return case.create_view()


def test_add_fault_distance_result_subset(rips_instance, initialize_test):
    case = rips_instance.project.load_case(GRID_PATH)
    view = _get_view(case)
    fault_collection = view.fault_collection()
    faults = fault_collection.faults()
    assert len(faults) > 0

    result = fault_collection.add_fault_distance_result("PY_FAULTDIST", faults[:1])
    assert result is not None
    assert result.result_name == "PY_FAULTDIST"


def test_add_fault_distance_result_all_matches_static(rips_instance, initialize_test):
    """Passing every fault into add_fault_distance_result must reproduce the
    static-native FAULTDIST values cell-for-cell."""
    case = rips_instance.project.load_case(GRID_PATH)
    view = _get_view(case)
    fault_collection = view.fault_collection()
    all_faults = fault_collection.faults()
    assert len(all_faults) > 0

    static_values = case.active_cell_property(
        rips.PropertyType.STATIC_NATIVE, "FAULTDIST", 0
    )

    fault_collection.add_fault_distance_result("PY_FAULTDIST_ALL", all_faults)

    generated_values = case.active_cell_property(
        rips.PropertyType.GENERATED, "PY_FAULTDIST_ALL", 0
    )

    assert len(static_values) == len(generated_values)
    assert len(static_values) > 0
    for s, g in zip(static_values, generated_values):
        assert math.isclose(s, g, abs_tol=1e-6)
