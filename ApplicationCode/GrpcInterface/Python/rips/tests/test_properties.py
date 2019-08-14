import sys
import os
import grpc
import pytest

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

import dataroot

def test_10kAsync(rips_instance, initializeTest):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.loadCase(path=casePath)

    resultChunks = case.properties.activeCellPropertyAsync('DYNAMIC_NATIVE', 'SOIL', 1)
    mysum = 0.0
    count = 0
    for chunk in resultChunks:
        mysum += sum(chunk.values)
        count += len(chunk.values)
    average = mysum / count
    assert(mysum == pytest.approx(621.768, abs=0.001))
    assert(average != pytest.approx(0.0158893, abs=0.0000001))
    assert(average == pytest.approx(0.0558893, abs=0.0000001))

def test_10kSync(rips_instance, initializeTest):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.loadCase(path=casePath)

    results = case.properties.activeCellProperty('DYNAMIC_NATIVE', 'SOIL', 1)
    mysum = sum(results)
    average = mysum / len(results)
    assert(mysum == pytest.approx(621.768, abs=0.001))
    assert(average != pytest.approx(0.0158893, abs=0.0000001))
    assert(average == pytest.approx(0.0558893, abs=0.0000001))

def test_10k_set(rips_instance, initializeTest):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.loadCase(path=casePath)

    results = case.properties.activeCellProperty('DYNAMIC_NATIVE', 'SOIL', 1)
    case.properties.setActiveCellProperty(results, 'GENERATED', 'SOIL', 1)

def test_10k_set_out_of_bounds(rips_instance, initializeTest):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.loadCase(path=casePath)

    results = case.properties.activeCellProperty('DYNAMIC_NATIVE', 'SOIL', 1)
    results.append(5.0)
    with pytest.raises(grpc.RpcError):
        assert case.properties.setActiveCellProperty(results, 'GENERATED', 'SOIL', 1)

def test_10k_set_out_of_bounds_client(rips_instance, initializeTest):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.loadCase(path=casePath)

    results = case.properties.activeCellProperty('DYNAMIC_NATIVE', 'SOIL', 1)
    case.properties.chunkSize = len(results)
    results.append(5.0)
    with pytest.raises(IndexError):
        assert case.properties.setActiveCellProperty(results, 'GENERATED', 'SOIL', 1)

def createResult(poroChunks, permxChunks):
    for (poroChunk, permxChunk) in zip(poroChunks, permxChunks):
        resultChunk = []
        for (poro, permx) in zip(poroChunk.values, permxChunk.values):
            resultChunk.append(poro * permx)
        yield resultChunk

def checkResults(poroValues, permxValues, poropermxValues):
    for (poro, permx, poropermx) in zip(poroValues, permxValues, poropermxValues):
            recalc = poro * permx
            assert(recalc == pytest.approx(poropermx, rel=1.0e-10))

def test_10k_PoroPermX(rips_instance, initializeTest):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.loadCase(path=casePath)

    poroChunks = case.properties.activeCellPropertyAsync('STATIC_NATIVE', 'PORO', 0)
    permxChunks = case.properties.activeCellPropertyAsync('STATIC_NATIVE', 'PERMX', 0)

    case.properties.setActiveCellPropertyAsync(createResult(poroChunks, permxChunks), 'GENERATED', 'POROPERMXAS', 0)

    poro = case.properties.activeCellProperty('STATIC_NATIVE', 'PORO', 0)
    permx = case.properties.activeCellProperty('STATIC_NATIVE', 'PERMX', 0)
    poroPermX = case.properties.activeCellProperty('GENERATED', 'POROPERMXAS', 0)

    checkResults(poro, permx, poroPermX)


    