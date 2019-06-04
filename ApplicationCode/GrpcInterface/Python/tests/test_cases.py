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