from rips.generated.generated_classes import ModeledWellPath, StimPlanModel, WellPathGeometry, WellPathTarget
import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

def test_well_path_target(rips_instance, initialize_test):
    well_path_coll = rips_instance.project.descendants(rips.WellPathCollection)[0]

    my_well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    my_well_path.name = "test"
    my_well_path.update()

    geometry = my_well_path.well_path_geometry()
    geometry.add_new_object(rips.WellPathTarget)
    geometry.add_new_object(rips.WellPathTarget)
    geometry.add_new_object(rips.WellPathTarget)
    assert len(geometry.well_path_targets()) == 3

    assert len(well_path_coll.well_paths()) == 1
    my_well_path_duplicate = well_path_coll.well_paths()[0]
    assert my_well_path_duplicate.name == "test"
    geometry_duplicate = my_well_path_duplicate.well_path_geometry()
    assert len(geometry_duplicate.well_path_targets()) == 3

    # Not allowed to add object of unrelated type
    invalid_object = geometry.add_new_object(rips.WellPath)
    assert invalid_object is None
    
