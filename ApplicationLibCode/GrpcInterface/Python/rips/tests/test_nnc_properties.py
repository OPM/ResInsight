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
    assert(len(properties) == 1)

    assert("TRAN" == properties[0].name)
    assert(NNCProperties_pb2.NNCPropertyType.Value('NNC_STATIC') == properties[0].property_type)

    nnc_connections = case.nnc_connections()
    assert(len(nnc_connections) == 3627)

    connection = nnc_connections[0]
    assert(connection.cell1.i == 33)
    assert(connection.cell1.j == 40)
    assert(connection.cell1.k == 14)
    assert(connection.cell_grid_index1 == 0)

    tran_vals = case.nnc_connections_static_values("TRAN")
    assert(len(tran_vals) == len(nnc_connections))

    for t in tran_vals:
        assert(isinstance(t, float))

    # Generate some data
    new_data = []
    for (c, _) in enumerate(nnc_connections):
        new_data.append(float(c))

    property_name = "NEW_PROP"
    case.set_nnc_connections_values(new_data, property_name, 0)
    new_prop_vals = case.nnc_connections_generated_values(property_name, 0)
    assert(len(new_prop_vals) == len(new_data))
    for i in range(0, len(new_data)):
        assert(new_data[i] == new_prop_vals[i])

    # Set some other data for second time step
    for i in range(0, len(new_data)):
        new_data[i] = new_data[i] * 2.0

    case.set_nnc_connections_values(new_data, property_name, 1)
    new_prop_vals = case.nnc_connections_generated_values(property_name, 1)
    assert(len(new_prop_vals) == len(nnc_connections))
    for i in range(0, len(new_data)):
        assert(new_data[i] == new_prop_vals[i])


def test_non_existing_dynamic_values(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)

    with pytest.raises(grpc.RpcError):
        case.nnc_connections_dynamic_values("x", 0)


def test_invalid_time_steps(rips_instance, initialize_test):
    casePath = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=casePath)
    with pytest.raises(grpc.RpcError):
        case.nnc_connections_generated_values("Formation Allan", 9999)
