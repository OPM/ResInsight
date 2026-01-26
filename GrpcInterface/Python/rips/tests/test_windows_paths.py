import sys
import os
import platform

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot

import pytest

pytestmark = pytest.mark.skipif(
    platform.system() != "Windows", reason="Windows-specific path test"
)


def test_import_well_paths_with_backslashes(rips_instance, initialize_test):
    """Test that Windows-style paths with backslashes are correctly handled.

    This test verifies the fix for issue #13435 where backslashes in Windows paths
    were incorrectly interpreted as escape characters when passed from Python to
    ResInsight via gRPC.
    """
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2

    # Build paths using forward slashes first, then convert to Windows-style backslashes
    well_path_a = os.path.abspath(
        os.path.join(case_root_path, "wellpath_a.dev")
    ).replace("/", "\\")
    well_path_b = os.path.abspath(
        os.path.join(case_root_path, "wellpath_b.dev")
    ).replace("/", "\\")

    # The paths now contain backslashes (e.g., "C:\path\to\wellpath_a.dev")
    # This tests the fix for issue #13435 where backslashes were incorrectly
    # interpreted as escape characters
    well_path_files = [well_path_a, well_path_b]

    well_path_names = rips_instance.project.import_well_paths(well_path_files)
    assert len(well_path_names) == 2
    wells = rips_instance.project.well_paths()
    assert len(wells) == 2
    assert wells[0].name == "Well Path A"
    assert wells[1].name == "Well Path B"
