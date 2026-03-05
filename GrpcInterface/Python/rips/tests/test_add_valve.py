import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips


def test_add_standalone_valve(rips_instance, initialize_test):
    well_path_coll = rips_instance.project.well_path_collection()

    # Create main well path using ModeledWellPath with targets
    main_well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    main_well_path.name = "main_well_check_connection"
    main_well_path.update()

    # Add geometry targets to main well path
    main_geometry = main_well_path.well_path_geometry()
    main_geometry.append_well_target([1000.0, 2000.0, 0.0])  # Surface
    main_geometry.append_well_target([1000.0, 2000.0, -100.0])  # 100m down
    main_geometry.append_well_target([1000.0, 2000.0, -300.0])  # 300m down
    main_geometry.append_well_target([1000.0, 2000.0, -500.0])  # 500m down
    main_geometry.update()

    valve = main_well_path.add_icv_valve(240.0)

    assert valve is not None

    assert valve.is_open

    assert valve.start_measured_depth == 240.0

    valve.is_open = False
    valve.start_measured_depth = 270.0
    valve.update()

    assert not valve.is_open
    assert valve.start_measured_depth == 270.0
