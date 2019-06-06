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


def createResult(poroChunks, permxChunks):
    for (poroChunk, permxChunk) in zip(poroChunks, permxChunks):
        resultChunk = []
        for (poro, permx) in zip(poroChunk.values, permxChunk.values):
            resultChunk.append(poro * permx)
        yield resultChunk

def checkResults(poroChunks, permxChunks, poroPermXChunks):
    for (poroChunk, permxChunk, poroPermXChunk) in zip(poroChunks, permxChunks, poroPermXChunks):
        for (poro, permx, poropermx) in zip(poroChunk.values, permxChunk.values, poroPermXChunk.values):
            recalc = poro * permx
            assert(recalc == pytest.approx(poropermx, rel=1.0e-10))

def test_10k_PoroPermX(rips_instance, initializeTest):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.loadCase(path=casePath)

    poroChunks = case.properties.activeCellProperty('STATIC_NATIVE', 'PORO', 0)
    permxChunks = case.properties.activeCellProperty('STATIC_NATIVE', 'PERMX', 0)

    case.properties.setActiveCellPropertyAsync(createResult(poroChunks, permxChunks), 'GENERATED', 'POROPERMXAS', 0)

    poroChunks = case.properties.activeCellProperty('STATIC_NATIVE', 'PORO', 0)
    permxChunks = case.properties.activeCellProperty('STATIC_NATIVE', 'PERMX', 0)
    poroPermXChunks = case.properties.activeCellProperty('GENERATED', 'POROPERMXAS', 0)

    checkResults(poroChunks, permxChunks, poroPermXChunks)


    