import sys
import os
import grpc
import pytest

import rips.generated.NNCProperties_pb2 as NNCProperties_pb2

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

import dataroot


def test_10kSync(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    properties = case.available_nnc_properties()
    assert(len(properties) == 3)

    assert("TRAN" == properties[0].name)
    assert(NNCProperties_pb2.NNCPropertyType.NNC_STATIC == properties[0].property_type)
    assert("Binary Formation Allen" == properties[1].name)
    assert(NNCProperties_pb2.NNCPropertyType.NNC_GENERATED == properties[1].property_type)
    assert("Formation Allen" == properties[2].name)
    assert(NNCProperties_pb2.NNCPropertyType.NNC_GENERATED == properties[2].property_type)    

    nnc_connections = case.nnc_connections()
    assert(len(nnc_connections) == 84759)

    connection = nnc_connections[0]
    assert(connection.cell1.i == 33)
    assert(connection.cell1.j == 40)
    assert(connection.cell1.k == 14)
    assert(connection.cell_grid_index1 == 0)
