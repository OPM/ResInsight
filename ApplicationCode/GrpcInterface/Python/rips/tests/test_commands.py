import sys
import os
import tempfile
import pytest
import grpc

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

import dataroot

def test_exportSnapshots(rips_instance, initialize_test):
    if not rips_instance.isGui():
        pytest.skip("Cannot run test without a GUI")

    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    rips_instance.project.load_case(case_path)
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        print("Temporary folder: ", tmpdirname)
        rips_instance.commands.set_export_folder(type='SNAPSHOTS', path=tmpdirname)
        rips_instance.commands.export_snapshots()
        print(os.listdir(tmpdirname))
        assert(len(os.listdir(tmpdirname)) > 0)
        for fileName in os.listdir(tmpdirname):
            assert(os.path.splitext(fileName)[1] == '.png')

def test_exportPropertyInView(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    rips_instance.project.load_case(case_path)
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        print("Temporary folder: ", tmpdirname)
        rips_instance.commands.set_export_folder(type='PROPERTIES', path=tmpdirname)
        case = rips_instance.project.case(id=0)
        rips_instance.commands.export_property_in_views(0, "3D View", 0)
        expected_file_name = case.name + "-" + str("3D_View") + "-" + "T0" + "-SOIL"
        full_path = tmpdirname + "/" + expected_file_name
        assert(os.path.exists(full_path))

@pytest.mark.skipif(sys.platform.startswith('linux'), reason="Brugge is currently exceptionally slow on Linux")
def test_loadGridCaseGroup(rips_instance, initialize_test):
     case_paths = []
     case_paths.append(dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID")
     case_paths.append(dataroot.PATH + "/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID")
     group_id, group_name = rips_instance.commands.create_grid_case_group(case_paths=case_paths)
     print(group_id, group_name)

def test_exportFlowCharacteristics(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    rips_instance.project.load_case(case_path)
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        print("Temporary folder: ", tmpdirname)
        file_name = tmpdirname + "/exportFlowChar.txt"
        rips_instance.commands.export_flow_characteristics(case_id=0, time_steps=8, producers=[], injectors = "I01", file_name = file_name)

def test_loadNonExistingCase(rips_instance, initialize_test):
    case_path = "Nonsense/Nonsense/Nonsense"
    with pytest.raises(grpc.RpcError):
        assert rips_instance.project.load_case(case_path)
