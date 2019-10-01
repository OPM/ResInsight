import sys
import os
import pytest
import grpc
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

import dataroot

def test_loadProject(rips_instance, initialize_test):
    project = rips_instance.project.open(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp")
    case = project.case(case_id=0)
    assert(case is not None)
    assert(case.name == "TEST10K_FLT_LGR_NNC")
    assert(case.case_id == 0)
    cases = rips_instance.project.cases()
    assert(len(cases) is 1)

@pytest.mark.skipif(sys.platform.startswith('linux'), reason="Brugge is currently exceptionally slow on Linux")
def test_loadGridCaseGroup(rips_instance, initialize_test):
     case_paths = []
     case_paths.append(dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID")
     case_paths.append(dataroot.PATH + "/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID")
     grid_case_group = rips_instance.project.create_grid_case_group(case_paths=case_paths)
     assert(grid_case_group is not None and grid_case_group.group_id == 0)

def test_exportSnapshots(rips_instance, initialize_test):
    if not rips_instance.is_gui():
        pytest.skip("Cannot run test without a GUI")

    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    rips_instance.project.load_case(case_path)
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        print("Temporary folder: ", tmpdirname)
        rips_instance.set_export_folder(export_type='SNAPSHOTS', path=tmpdirname)
        rips_instance.project.export_snapshots()
        print(os.listdir(tmpdirname))
        assert(len(os.listdir(tmpdirname)) > 0)
        for fileName in os.listdir(tmpdirname):
            assert(os.path.splitext(fileName)[1] == '.png')
