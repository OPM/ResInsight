import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], '..'))
import rips

import dataroot

def test_Launch(rips_instance):
    assert(rips_instance is not None)

def test_EmptyProject(rips_instance):
    cases = rips_instance.project.cases()
    assert(len(cases) is 0)

def test_OneCase(rips_instance):
    case = rips_instance.project.loadCase(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID")
    cases = rips_instance.project.cases()
    assert(len(cases) is 1)

def test_MultipleCases(rips_instance):
    casePaths = []
    casePaths.append(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID")
    casePaths.append(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID")
    casePaths.append(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID")
    casePaths.append(dataroot.PATH + "/case_with_10_timesteps/Real0/BRUGGE_0000.EGRID")
    casePaths.append(dataroot.PATH + "/case_with_10_timesteps/Real10/BRUGGE_0010.EGRID")
    casePaths.append(dataroot.PATH + "/case_with_10_timesteps/Real20/BRUGGE_0020.EGRID")

    caseNames = []
    for casePath in casePaths:
        caseName = os.path.splitext(os.path.basename(casePath))[0]
        caseNames.append(caseName)
        rips_instance.project.loadCase(path=casePath)

    print(caseNames)
    cases = rips_instance.project.cases()
    for case in cases:
        print (case.index, case.name)
    assert(len(cases) == len(caseNames))
    for i, caseName in enumerate(caseNames):
        print(i, caseName)
        assert(caseName == cases[i].name)
