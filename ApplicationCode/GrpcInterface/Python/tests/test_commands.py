import sys
import os
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], '..'))
import rips

import dataroot

def test_exportSnapshots(rips_instance, initializeTest):
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