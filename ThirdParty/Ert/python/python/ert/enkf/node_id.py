from ctypes import Structure, c_int
from cwrap import Prototype

class NodeId(Structure):
    """
    NodeId is specified in enkf_types.h
    """
    _fields_ = [("report_step", c_int),
                ("iens", c_int)]

    def __init__(self, report_step, realization_number):
        """
        @type report_step: int
        @type realization_number: int
        """
        super(NodeId, self).__init__(report_step, realization_number)

    def __repr__(self):
        rs = self.report_step
        ie = self.iens
        return 'NodeId(report_step = %d, iens = %d)' % (rs, ie)

Prototype.registerType( "node_id" , NodeId )
