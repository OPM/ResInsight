from cwrap import BaseCClass, CWrapper
from ert.well import ECL_WELL_LIB

class WellConnection(BaseCClass):

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly")


    def isOpen(self):
        """ @rtype: bool """
        return WellConnection.cNamespace().is_open(self)


    def ijk(self):
        """ @rtype: tuple of (int, int, int) """
        i = WellConnection.cNamespace().i(self)
        j = WellConnection.cNamespace().j(self)
        k = WellConnection.cNamespace().k(self)
        return i, j, k

    def direction(self):
        """ @rtype: WellConnectionDirectionEnum """
        return WellConnection.cNamespace().get_dir(self)

    def segmentId(self):
        """ @rtype: int """
        return WellConnection.cNamespace().segment_id(self)

    def isFractureConnection(self):
        """ @rtype: bool """
        return WellConnection.cNamespace().fracture_connection(self)

    def isMatrixConnection(self):
        """ @rtype: bool """
        return WellConnection.cNamespace().matrix_connection(self)

    def connectionFactor(self):
        """ @rtype: float """
        return WellConnection.cNamespace().connection_factor(self)

    def __eq__(self, other):
        return WellConnection.cNamespace().equal(self, other)

    def __ne__(self, other):
        return not self == other

    def free(self):
        pass

    def isMultiSegmentWell(self):
        """ @rtype: bool """
        return WellConnection.cNamespace().is_msw(self)


CWrapper.registerObjectType("well_connection", WellConnection)
cwrapper = CWrapper(ECL_WELL_LIB)


WellConnection.cNamespace().i = cwrapper.prototype("int well_conn_get_i(well_connection)")
WellConnection.cNamespace().j = cwrapper.prototype("int well_conn_get_j(well_connection)")
WellConnection.cNamespace().k = cwrapper.prototype("int well_conn_get_k(well_connection)")
WellConnection.cNamespace().get_dir = cwrapper.prototype("well_connection_dir_enum well_conn_get_dir(well_connection)")

WellConnection.cNamespace().segment_id = cwrapper.prototype("int well_conn_get_segment_id(well_connection)")
WellConnection.cNamespace().is_open = cwrapper.prototype("bool well_conn_open(well_connection)")
WellConnection.cNamespace().is_msw = cwrapper.prototype("bool well_conn_MSW(well_connection)")
WellConnection.cNamespace().fracture_connection = cwrapper.prototype("bool well_conn_fracture_connection(well_connection)")
WellConnection.cNamespace().matrix_connection = cwrapper.prototype("bool well_conn_matrix_connection(well_connection)")
WellConnection.cNamespace().connection_factor = cwrapper.prototype("double well_conn_get_connection_factor(well_connection)")

WellConnection.cNamespace().equal = cwrapper.prototype("bool well_conn_equal(well_connection, well_connection)")
