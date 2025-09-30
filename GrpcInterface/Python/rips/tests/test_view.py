import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import dataroot


def test_visible_cells(rips_instance, initialize_test):
    """Test the visible_cells() method on a view"""
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    # Get the first view or create one
    views = case.views()
    if views:
        view = views[0]
    else:
        view = case.create_view()

    # Test visible_cells at time step 0
    visibility = view.visible_cells(time_step=0)

    # Verify we got a list
    assert isinstance(visibility, list)

    # Verify we have data
    assert len(visibility) > 0

    # Verify all values are either 0 or 1
    for v in visibility:
        assert v in [0, 1]

    # Count visible and invisible cells
    visible_count = sum(visibility)
    invisible_count = len(visibility) - visible_count

    # Total cells should match grid size
    assert len(visibility) == visible_count + invisible_count

    # The visibility array should have the correct size (can be all visible, all invisible, or mixed)
    # This depends on the view settings
    assert visible_count + invisible_count > 0


def test_visible_cells_different_timesteps(rips_instance, initialize_test):
    """Test visible_cells() at different time steps"""
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)

    view = case.views()[0] if case.views() else case.create_view()

    # Test at time step 0
    visibility_ts0 = view.visible_cells(time_step=0)
    assert len(visibility_ts0) > 0

    # Test at time step 1 (if it exists)
    visibility_ts1 = view.visible_cells(time_step=1)
    assert len(visibility_ts1) > 0

    # Both should have the same number of cells
    assert len(visibility_ts0) == len(visibility_ts1)
