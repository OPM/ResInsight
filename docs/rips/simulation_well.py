"""
ResInsight SimulationWell
"""
import grpc

import rips.generated.SimulationWell_pb2 as SimulationWell_pb2
import rips.generated.SimulationWell_pb2_grpc as SimulationWell_pb2_grpc

import rips.generated.Properties_pb2 as Properties_pb2
import rips.generated.Properties_pb2_grpc as Properties_pb2_grpc

import rips.generated.Commands_pb2 as Cmd
from rips.generated.pdm_objects import SimulationWell

from rips.pdmobject import PdmObject, add_method
import rips.case

@add_method(SimulationWell)
def __custom_init__(self, pb2_object, channel):
    self._simulation_well_stub = SimulationWell_pb2_grpc.SimulationWellStub(channel)

@add_method(SimulationWell)
def status(self, timestep):
    sim_well_request = SimulationWell_pb2.SimulationWellRequest(case_id=self.case().id,
                                                                well_name=self.name,
                                                                timestep=timestep)
    return self._simulation_well_stub.GetSimulationWellStatus(sim_well_request)

@add_method(SimulationWell)
def cells(self, timestep):
    sim_well_request = SimulationWell_pb2.SimulationWellRequest(case_id=self.case().id,
                                                                well_name=self.name,
                                                                timestep=timestep)
    return self._simulation_well_stub.GetSimulationWellCells(sim_well_request).data

@add_method(SimulationWell)
def case(self):
    return self.ancestor(rips.case.Case)
