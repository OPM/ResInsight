from cwrap import BaseCClass

from ecl import EclPrototype
from ecl.well import WellTypeEnum, WellConnection
from ecl.util.util import CTime

class WellState(BaseCClass):
    TYPE_NAME = "well_state"

    _global_connections_size = EclPrototype("int well_conn_collection_get_size(void*)", bind = False)
    _global_connections_iget = EclPrototype("well_connection_ref well_conn_collection_iget(void*, int)", bind = False)
    _segment_collection_size = EclPrototype("int well_segment_collection_get_size(void*)", bind = False)
    _segment_collection_iget = EclPrototype("well_segment_ref well_segment_collection_iget(void*, int)", bind = False)
    _has_global_connections  = EclPrototype("bool  well_state_has_global_connections(well_state)")
    _get_global_connections  = EclPrototype("void* well_state_get_global_connections(well_state)")
    _get_segment_collection  = EclPrototype("void* well_state_get_segments(well_state)")
    _branches                = EclPrototype("void* well_state_get_branches(well_state)")
    _segments                = EclPrototype("void* well_state_get_segments(well_state)")
    _get_name                = EclPrototype("char* well_state_get_name(well_state)")
    _is_open                 = EclPrototype("bool  well_state_is_open(well_state)")
    _is_msw                  = EclPrototype("bool  well_state_is_MSW(well_state)")
    _well_number             = EclPrototype("int   well_state_get_well_nr(well_state)")
    _report_number           = EclPrototype("int   well_state_get_report_nr(well_state)")
    _has_segment_data        = EclPrototype("bool  well_state_has_segment_data(well_state)")
    _sim_time                = EclPrototype("time_t well_state_get_sim_time(well_state)")
    _well_type               = EclPrototype("well_type_enum well_state_get_type(well_state)")
    _oil_rate                = EclPrototype("double well_state_get_oil_rate(well_state)")
    _gas_rate                = EclPrototype("double well_state_get_gas_rate(well_state)")
    _water_rate              = EclPrototype("double well_state_get_water_rate(well_state)")
    _volume_rate             = EclPrototype("double well_state_get_volume_rate(well_state)")
    _oil_rate_si             = EclPrototype("double well_state_get_oil_rate_si(well_state)")
    _gas_rate_si             = EclPrototype("double well_state_get_gas_rate_si(well_state)")
    _water_rate_si           = EclPrototype("double well_state_get_water_rate_si(well_state)")
    _volume_rate_si          = EclPrototype("double well_state_get_volume_rate_si(well_state)")
    _get_global_well_head    = EclPrototype("well_connection_ref well_state_get_global_wellhead(well_state)")
    
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly")

    def name(self):
        """ @rtype: str """
        return self._get_name( )

    def isOpen(self):
        """ @rtype: bool """
        return self._is_open( )

    def free(self):
        pass

    def wellHead(self):
        well_head = self._get_global_well_head()
        well_head.setParent( self )
        return well_head

    def wellNumber(self):
        """ @rtype: int """
        return self._well_number( )

    def reportNumber(self):
        """ @rtype: int """
        return self._report_number( )

    def simulationTime(self):
        """ @rtype: CTime """
        return self._sim_time( )

    def wellType(self):
        """ @rtype: WellTypeEnum """
        return self._well_type( )

    def hasGlobalConnections(self):
        """ @rtype: bool """
        return self._has_global_connections( )

    def globalConnections(self):
        """ @rtype: list of WellConnection """
        global_connections = self._get_global_connections( )
        count = self._global_connections_size( global_connections )

        values = []
        for index in range(count):
            value = self._global_connections_iget(global_connections, index).setParent( self )
            values.append(value)
        return values

    def __len__(self):
        return self.numSegments()

    def __getitem__(self, idx):
        return self.igetSegment(idx)

    def numSegments(self):
        """ @rtype: int """
        segment_collection = self._get_segment_collection( )
        count = self._segment_collection_size(segment_collection)
        return count


    def segments(self):
        """ @rtype: list of WellSegment """
        segment_collection = self._get_segment_collection( )

        values = []
        for index in range(self.numSegments()):
            value = self._segment_collection_iget(segment_collection, index).setParent(self)
            values.append(value)

        return values


    def igetSegment(self , seg_idx):
        """ @rtype: WellSegment """
        if seg_idx < 0:
            seg_idx += len(self)

        if not 0 <= seg_idx < self.numSegments():
            raise IndexError("Invalid index:%d - valid range [0,%d)" % (seg_idx , len(self)))

        segment_collection = self._get_segment_collection( )
        return self._segment_collection_iget(segment_collection, seg_idx).setParent(self)

    def isMultiSegmentWell(self):
        """ @rtype: bool """
        return self._is_msw( )

    def hasSegmentData(self):
        """ @rtype: bool """
        return self._has_segment_data( )

    def __repr__(self):
        name = self.name()
        if name:
            name = '%s' % name
        else:
            name = '[no name]'
        msw  = ' (multi segment)' if self.isMultiSegmentWell() else ''
        wn = str(self.wellNumber())
        type_ = self.wellType()
        open_ = 'open' if self.isOpen() else 'shut'
        cnt = '%s%s, number = %s, type = "%s", state = %s' % (name, msw, wn, type_, open_)
        return self._create_repr(cnt)

    def gasRate(self):
        return self._gas_rate( )

    def waterRate(self):
        return self._water_rate( )

    def oilRate(self):
        return self._oil_rate( )

    def volumeRate(self):
        return self._volume_rate( )

    def gasRateSI(self):
        return self._gas_rate_si( )

    def waterRateSI(self):
        return self._water_rate_si( )

    def oilRateSI(self):
        return self._oil_rate_si( )

    def volumeRateSI(self):
        return self._volume_rate_si( )
