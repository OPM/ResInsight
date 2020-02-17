"""
ResInsight SimulationWell
"""
import grpc

import rips.generated.SimulationWell_pb2 as SimulationWell_pb2
import rips.generated.SimulationWell_pb2_grpc as SimulationWell_pb2_grpc

import rips.generated.Properties_pb2 as Properties_pb2
import rips.generated.Properties_pb2_grpc as Properties_pb2_grpc

import rips.generated.Commands_pb2 as Cmd

from rips.pdmobject import PdmObject

class SimulationWell(PdmObject):
    """ResInsight simulation well class

    Attributes:
        name(string): Name of the well.

    """
    def __init__(self, name, case_id, pdm_object):
        PdmObject.__init__(self, pdm_object.pb2_object(), pdm_object.channel(), pdm_object.project())
        self._simulation_well_stub = SimulationWell_pb2_grpc.SimulationWellStub(pdm_object.channel())
        self.name = name
        self.case_id = case_id

    def status(self, timestep):
        sim_well_request = SimulationWell_pb2.SimulationWellRequest(case_id=self.case_id,
                                                                    well_name=self.name,
                                                                    timestep=timestep)
        return self._simulation_well_stub.GetSimulationWellStatus(sim_well_request)

    def cells(self, timestep):
        sim_well_request = SimulationWell_pb2.SimulationWellRequest(case_id=self.case_id,
                                                                    well_name=self.name,
                                                                    timestep=timestep)
        return self._simulation_well_stub.GetSimulationWellCells(sim_well_request).data
