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
    """Get simulation well status

     **SimulationWellStatus class description**::

        Parameter   | Description                                                   | Type
        ----------- | ------------------------------------------------------------- | -----
        well_type   | Well type as string                                           | string
        is_open     | True if simulation well is open at the specified time step    | bool 

    Arguments:
        timestep(int): Time step index

    """
    sim_well_request = SimulationWell_pb2.SimulationWellRequest(case_id=self.case().id,
                                                                well_name=self.name,
                                                                timestep=timestep)
    return self._simulation_well_stub.GetSimulationWellStatus(sim_well_request)

@add_method(SimulationWell)
def cells(self, timestep):
    """Get reservoir cells the simulation well is defined for

     **SimulationWellCellInfo class description**::

        Parameter   | Description                                               | Type
        ----------- | --------------------------------------------------------- | -----
        ijk         | Cell IJK location                                         | Vec3i
        grid_index  | Grid index                                                | int
        is_open     | True if connection to is open at the specified time step  | bool 
        branch_id   |                                                           | int
        segment_id  |                                                           | int

    Arguments:
        timestep(int): Time step index

    Returns:
        List of SimulationWellCellInfo

    """
    sim_well_request = SimulationWell_pb2.SimulationWellRequest(case_id=self.case().id,
                                                                well_name=self.name,
                                                                timestep=timestep)
    return self._simulation_well_stub.GetSimulationWellCells(sim_well_request).data

@add_method(SimulationWell)
def case(self):
    return self.ancestor(rips.case.Case)
