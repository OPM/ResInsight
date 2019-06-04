import sys
import os
import pytest

sys.path.insert(1, os.path.join(sys.path[0], '..'))
import rips

import dataroot

def test_10k(rips_instance, initializeTest):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.loadCase(path=casePath)

    resultChunks = case.properties.activeCellProperty('DYNAMIC_NATIVE', 'SOIL', 1)
    mysum = 0.0
    count = 0
    for chunk in resultChunks:
        mysum += sum(chunk.values)
        count += len(chunk.values)
    average = mysum / count
    assert(mysum == pytest.approx(621.768, abs=0.001))
    assert(average != pytest.approx(0.0158893, abs=0.0000001))
    assert(average == pytest.approx(0.0558893, abs=0.0000001))
