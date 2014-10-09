from ctypes import Structure, c_int
from ert.cwrap import CWrapper
from ert.enkf.enums import EnkfStateType

class NodeId(Structure):
    """
    NodeId is specified in enkf_types.h
    """
    _fields_ = [("report_step", c_int),
                ("iens", c_int),
                ("state", c_int)]

    def __init__(self, report_step, realization_number, state):
        """
        @type report_step: int
        @type realization_number: int
        @type state: EnkfStateType
        """
        super(NodeId, self).__init__(report_step, realization_number, state)


CWrapper.registerType("node_id", NodeId)

