import sys
import os
import pytest 

sys.path.insert(1, os.path.join(sys.path[0], '..'))
import rips

import dataroot

def test_loadProject(rips_instance, initializeTest):
    project = rips_instance.project.open(dataroot.PATH + "/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp")
    case = project.case(id=0)
    assert(case is not None)
    assert(case.name == "TEST10K_FLT_LGR_NNC")
    assert(case.id == 0)
    cases = rips_instance.project.cases()
    assert(len(cases) is 1)