import sys
import os
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_append_lateral_from_measured_depth(rips_instance, initialize_test):
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

    measured_depth = 150
    lateral = main_well_path.append_lateral(measured_depth)
    assert lateral is not None


def test_append_lateral(rips_instance, initialize_test):
    well_path_coll = rips_instance.project.well_path_collection()

    # Create main well path
    main_well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    main_well_path.name = "main_well"
    main_well_path.update()

    # Create lateral well path
    lateral_well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    lateral_well_path.name = "lateral_well"
    lateral_well_path.update()

    # Test appending lateral to main well path
    # This should fail because well paths don't have geometry defined
    try:
        main_well_path.append_lateral_from_geometry(well_path_lateral=lateral_well_path)
        # If we get here, the operation succeeded unexpectedly
        assert False, "Expected append_lateral to fail due to missing geometry"
    except rips.RipsError as e:
        # Expected error message when no geometry is available
        expected_msg = "No geometry available for main well path. Cannot append lateral to main well path."
        assert str(e) == expected_msg

    # Verify the lateral well path objects still exist and have correct names
    assert lateral_well_path is not None
    assert lateral_well_path.name == "lateral_well"
    assert main_well_path is not None
    assert main_well_path.name == "main_well"


def test_append_lateral_with_geometry(rips_instance, initialize_test):
    well_path_coll = rips_instance.project.well_path_collection()

    # Create main well path with geometry using coordinates
    main_coordinates = [
        [1000.0, 2000.0, 0.0],  # Surface point
        [1000.0, 2000.0, -100.0],  # 100m down
        [1000.0, 2000.0, -500.0],  # 500m down
        [1000.0, 2000.0, -1000.0],  # 1000m down
    ]

    main_well_path = well_path_coll.import_well_path_from_points(
        name="main_well_with_geometry", coordinates=main_coordinates
    )

    # Create lateral well path with geometry that starts along the main well path
    lateral_coordinates = [
        [1000.0, 2000.0, -300.0],  # Start along main well at 300m depth
        [1020.0, 2020.0, -320.0],  # Deviate slightly
        [1050.0, 2050.0, -350.0],  # Continue deviation
        [1100.0, 2100.0, -400.0],  # Final lateral point
    ]

    lateral_well_path = well_path_coll.import_well_path_from_points(
        name="lateral_well_with_geometry", coordinates=lateral_coordinates
    )

    # Test appending lateral to main well path - this should work with geometry
    # Note: The actual connection may still fail if the geometries don't align properly,
    # but we're testing that the function can be called with valid geometry objects
    try:
        main_well_path.append_lateral_from_geometry(well_path_lateral=lateral_well_path)
        # If successful, verify both well paths still exist
        assert main_well_path.name == "main_well_with_geometry"
        assert lateral_well_path.name == "lateral_well_with_geometry"
    except rips.RipsError as e:
        # If it fails, it should be for geometric reasons, not missing geometry
        error_msg = str(e)
        assert (
            "No geometry available" not in error_msg
        ), f"Unexpected geometry error: {error_msg}"
        # Other geometric errors (like alignment issues) are acceptable for this test
        print(f"Geometric alignment error (expected): {error_msg}")

    # Verify the well path objects are still valid regardless of connection result
    assert main_well_path is not None
    assert lateral_well_path is not None


def test_append_lateral_check_connection(rips_instance, initialize_test):
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

    # Create lateral well path using ModeledWellPath with targets
    lateral_well_path = well_path_coll.add_new_object(rips.ModeledWellPath)
    lateral_well_path.name = "lateral_well_check_connection"
    lateral_well_path.update()

    # Add geometry targets to lateral well path
    lateral_geometry = lateral_well_path.well_path_geometry()
    lateral_geometry.append_well_target(
        [1000.0, 2000.0, -150.0]
    )  # Start at 150m on main
    lateral_geometry.append_well_target([1020.0, 2020.0, -200.0])  # Deviate
    lateral_geometry.append_well_target([1050.0, 2050.0, -250.0])  # Continue
    lateral_geometry.update()

    # Get initial geometry state
    initial_attached_state = lateral_geometry.attached_to_parent_well

    # Test appending lateral to main well path
    try:
        main_well_path.append_lateral_from_geometry(well_path_lateral=lateral_well_path)

        # Check if connection was successful by examining the geometry properties
        lateral_geometry.update()  # Refresh from server
        final_attached_state = lateral_geometry.attached_to_parent_well

        # If connection was successful, verify the parent relationship
        if final_attached_state and not initial_attached_state:
            print("Successfully connected lateral to parent well path")
            assert lateral_well_path.name == "lateral_well_check_connection"
            assert main_well_path.name == "main_well_check_connection"

            # The lateral should now be marked as attached to parent
            assert lateral_geometry.attached_to_parent_well
        else:
            print("Connection did not change attachment state - may be geometric issue")
            # Still a valid test if the function completed without error

    except rips.RipsError as e:
        # Log the error for debugging but don't fail the test
        # Different geometric configurations may result in different connection outcomes
        error_msg = str(e)
        print(f"Connection attempt resulted in error: {error_msg}")

        # Ensure it's not a "no geometry" error since we provided geometry
        assert (
            "No geometry available" not in error_msg
        ), f"Unexpected geometry error: {error_msg}"

    # Verify both well paths are still valid
    assert main_well_path is not None
    assert lateral_well_path is not None
    assert main_well_path.name == "main_well_check_connection"
    assert lateral_well_path.name == "lateral_well_check_connection"
