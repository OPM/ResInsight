"""
ResInsight SimulationWell
"""
import grpc

import SimulationWell_pb2
import SimulationWell_pb2_grpc

import Properties_pb2
import Properties_pb2_grpc
import PdmObject_pb2

from .resinsight_classes import SimulationWell

from .case import Case
from .pdmobject import PdmObjectBase, add_method

from typing import List, Optional


@add_method(SimulationWell)
def __custom_init__(
    self: SimulationWell, pb2_object: PdmObject_pb2.PdmObject, channel: grpc.Channel
) -> None:
    self.__simulation_well_stub = SimulationWell_pb2_grpc.SimulationWellStub(channel)


@add_method(SimulationWell)
def status(
    self: SimulationWell, timestep: int
) -> List[SimulationWell_pb2.SimulationWellStatus]:
    """Get simulation well status

     **SimulationWellStatus class description**::

        Parameter   | Description                                                   | Type
        ----------- | ------------------------------------------------------------- | -----
        well_type   | Well type as string                                           | string
        is_open     | True if simulation well is open at the specified time step    | bool

    Arguments:
        timestep(int): Time step index

    """
    sim_well_request = SimulationWell_pb2.SimulationWellRequest(
        case_id=self.case().id, well_name=self.name, timestep=timestep
    )
    return self.__simulation_well_stub.GetSimulationWellStatus(sim_well_request)


@add_method(SimulationWell)
def cells(
    self: SimulationWell, timestep: int
) -> List[SimulationWell_pb2.SimulationWellCellInfo]:
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
    sim_well_request = SimulationWell_pb2.SimulationWellRequest(
        case_id=self.case().id, well_name=self.name, timestep=timestep
    )
    return self.__simulation_well_stub.GetSimulationWellCells(sim_well_request).data


@add_method(SimulationWell)
def case(self: SimulationWell) -> Optional[Case]:
    return self.ancestor(Case)
