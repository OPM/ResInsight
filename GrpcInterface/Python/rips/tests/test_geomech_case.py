import sys
import os
import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

GEOMECH_MODEL = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "../../../../ApplicationLibCode/UnitTests/TestData/RifVtkReader/model.pvd",
)


def test_geomech_case_results(rips_instance, initialize_test):
    case = rips_instance.project.load_case(GEOMECH_MODEL)
    assert isinstance(case, rips.GeoMechCase)

    # The steps of this model are not dates
    assert len(case.time_steps()) == 0
    assert len(case.time_step_names()) == 2

    assert case.cell_count().active_cell_count == 2
    assert "Stress" in case.available_properties("GEOMECH_ELEMENT")
    values = case.active_cell_property("GEOMECH_ELEMENT", "Stress", 1)
    assert values == pytest.approx([150.0, 250.0])

    cell_info = case.cell_info_for_active_cells()
    assert len(cell_info) == 2
    assert all(c.grid_index == 0 for c in cell_info)
