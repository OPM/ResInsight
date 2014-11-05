from ert.cwrap import BaseCClass, CWrapper
from ert.well import ECL_WELL_LIB, WellTypeEnum, WellConnection
from ert.util import CTime


class WellState(BaseCClass):

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly")


    def name(self):
        """ @rtype: str """
        return WellState.cNamespace().get_name(self)

    def isOpen(self):
        """ @rtype: bool """
        return WellState.cNamespace().is_open(self)

    def free(self):
        pass

    def wellNumber(self):
        """ @rtype: int """
        return WellState.cNamespace().well_number(self)

    def reportNumber(self):
        """ @rtype: int """
        return WellState.cNamespace().report_number(self)

    def simulationTime(self):
        """ @rtype: CTime """
        return WellState.cNamespace().sim_time(self)

    def wellType(self):
        """ @rtype: WellTypeEnum """
        return WellState.cNamespace().well_type(self)

    def hasGlobalConnections(self):
        """ @rtype: bool """
        return WellState.cNamespace().has_global_connections(self)

    def globalConnections(self):
        """ @rtype: list of WellConnection """
        global_connections = WellState.cNamespace().get_global_connections(self)
        count = WellState.cNamespace().global_connections_size(global_connections)

        values = []
        for index in range(count):
            value = WellState.cNamespace().global_connections_iget(global_connections, index).setParent(self)
            values.append(value)
        return values


    def numSegments(self):
        """ @rtype: int """
        segment_collection = WellState.cNamespace().get_segment_collection(self)
        count = WellState.cNamespace().segment_collection_size(segment_collection)
        return count


    def segments(self):
        """ @rtype: list of WellSegment """
        segment_collection = WellState.cNamespace().get_segment_collection(self)

        values = []
        for index in range(self.numSegments()):
            value = WellState.cNamespace().segment_collection_iget(segment_collection, index).setParent(self)
            values.append(value)

        return values


    def igetSegment(self , segment_index):
        """ @rtype: WellSegment """
        if segment_index < 0:
            segment_index += len(self)
        
        if not 0 <= segment_index < self.numSegments():
            raise IndexError("Invalid index:%d - valid range [0,%d)" % (index , len(self)))
        
        segment_collection = WellState.cNamespace().get_segment_collection(self)
        return WellState.cNamespace().segment_collection_iget(segment_collection, segment_index).setParent(self)



    # def branches(self):
    #     """ @rtype: BranchCollection """

    def isMultiSegmentWell(self):
        """ @rtype: bool """
        return WellState.cNamespace().is_msw(self)

    def hasSegmentData(self):
        """ @rtype: bool """
        return WellState.cNamespace().has_segment_data(self)


CWrapper.registerObjectType("well_state", WellState)
cwrapper = CWrapper(ECL_WELL_LIB)


WellState.cNamespace().get_name = cwrapper.prototype("char* well_state_get_name(well_state)")
WellState.cNamespace().is_open = cwrapper.prototype("bool well_state_is_open(well_state)")
WellState.cNamespace().is_msw = cwrapper.prototype("bool well_state_is_MSW(well_state)")
WellState.cNamespace().well_number = cwrapper.prototype("int well_state_get_well_nr(well_state)")
WellState.cNamespace().report_number = cwrapper.prototype("int well_state_get_report_nr(well_state)")
WellState.cNamespace().sim_time = cwrapper.prototype("time_t well_state_get_sim_time(well_state)")
WellState.cNamespace().well_type = cwrapper.prototype("well_type_enum well_state_get_type(well_state)")
WellState.cNamespace().has_segment_data = cwrapper.prototype("bool well_state_has_segment_data(well_state)")

WellState.cNamespace().has_global_connections = cwrapper.prototype("bool well_state_has_global_connections(well_state)")
WellState.cNamespace().get_global_connections = cwrapper.prototype("c_void_p well_state_get_global_connections(well_state)")
WellState.cNamespace().global_connections_size = cwrapper.prototype("int well_conn_collection_get_size(c_void_p)")
WellState.cNamespace().global_connections_iget = cwrapper.prototype("well_connection_ref well_conn_collection_iget(c_void_p, int)")


WellState.cNamespace().get_segment_collection = cwrapper.prototype("c_void_p well_state_get_segments(well_state)")
WellState.cNamespace().segment_collection_size = cwrapper.prototype("int well_segment_collection_get_size(c_void_p)")
WellState.cNamespace().segment_collection_iget = cwrapper.prototype("well_segment_ref well_segment_collection_iget(c_void_p, int)")


WellState.cNamespace().branches = cwrapper.prototype("c_void_p well_state_get_branches(well_state)")
WellState.cNamespace().segments = cwrapper.prototype("c_void_p well_state_get_segments(well_state)")

