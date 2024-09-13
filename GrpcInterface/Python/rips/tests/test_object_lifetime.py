import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips


def test_well_path(rips_instance, initialize_test):
    well_path_coll = rips_instance.project.descendants(rips.WellPathCollection)[0]
    assert len(well_path_coll.well_paths()) == 0

    well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    well_path2 = well_path_coll.add_new_object(rips.ModeledWellPath)
    assert len(well_path_coll.well_paths()) == 2

    well_path.delete()
    assert len(well_path_coll.well_paths()) == 1

    try:
        # Delete again should throw exception
        well_path.delete()
        assert False
    except Exception:
        assert True

    well_path2.delete()
    assert len(well_path_coll.well_paths()) == 0
