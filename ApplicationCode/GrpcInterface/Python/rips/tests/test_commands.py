import sys
import os
import tempfile
import pytest

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

import dataroot

def test_exportSnapshots(rips_instance, initializeTest):
    if not rips_instance.app.isGui():
        pytest.skip("Cannot run test without a GUI")

    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    rips_instance.project.loadCase(casePath)
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        print("Temporary folder: ", tmpdirname)
        rips_instance.commands.setExportFolder(type='SNAPSHOTS', path=tmpdirname)
        rips_instance.commands.exportSnapshots()
        print(os.listdir(tmpdirname))
        assert(len(os.listdir(tmpdirname)) > 0)
        for fileName in os.listdir(tmpdirname):
            assert(os.path.splitext(fileName)[1] == '.png')

def test_exportPropertyInView(rips_instance, initializeTest):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    rips_instance.project.loadCase(casePath)
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        print("Temporary folder: ", tmpdirname)
        rips_instance.commands.setExportFolder(type='PROPERTIES', path=tmpdirname)
        case = rips_instance.project.case(id=0)
        rips_instance.commands.exportPropertyInViews(0, "3D View", 0)
        expectedFileName = case.name + "-" + str("3D_View") + "-" + "T0" + "-SOIL"
        fullPath = tmpdirname + "/" + expectedFileName
        assert(os.path.exists(fullPath))

@pytest.mark.skipif(sys.platform.startswith('linux'), reason="Brugge is currently exceptionally slow on Linux")
def test_loadGridCaseGroup(rips_instance, initializeTest):
     casePaths = []
     casePaths.append(dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID")
     casePaths.append(dataroot.PATH + "/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID")
     groupId, groupName = rips_instance.commands.createGridCaseGroup(casePaths=casePaths)
     print(groupId, groupName)

def test_exportFlowCharacteristics(rips_instance, initializeTest):
    casePath = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    rips_instance.project.loadCase(casePath)
    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        print("Temporary folder: ", tmpdirname)
        fileName = tmpdirname + "/exportFlowChar.txt"
        rips_instance.commands.exportFlowCharacteristics(caseId=0, timeSteps=8, producers=[], injectors = "I01", fileName = fileName)