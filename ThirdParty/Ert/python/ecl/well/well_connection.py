from cwrap import BaseCClass
from ecl import EclPrototype
from ecl.well import WellConnectionDirectionEnum

class WellConnection(BaseCClass):
    TYPE_NAME = "well_connection"

    _i                   = EclPrototype("int    well_conn_get_i(well_connection)")
    _j                   = EclPrototype("int    well_conn_get_j(well_connection)")
    _k                   = EclPrototype("int    well_conn_get_k(well_connection)")
    _segment_id          = EclPrototype("int    well_conn_get_segment_id(well_connection)")
    _is_open             = EclPrototype("bool   well_conn_open(well_connection)")
    _is_msw              = EclPrototype("bool   well_conn_MSW(well_connection)")
    _fracture_connection = EclPrototype("bool   well_conn_fracture_connection(well_connection)")
    _matrix_connection   = EclPrototype("bool   well_conn_matrix_connection(well_connection)")
    _connection_factor   = EclPrototype("double well_conn_get_connection_factor(well_connection)")
    _equal               = EclPrototype("bool   well_conn_equal(well_connection, well_connection)")
    _get_dir             = EclPrototype("void*  well_conn_get_dir(well_connection)")
    _oil_rate            = EclPrototype("double well_conn_get_oil_rate(well_connection)")
    _gas_rate            = EclPrototype("double well_conn_get_gas_rate(well_connection)")
    _water_rate          = EclPrototype("double well_conn_get_water_rate(well_connection)")
    _volume_rate         = EclPrototype("double well_conn_get_volume_rate(well_connection)")

    _oil_rate_si         = EclPrototype("double well_conn_get_oil_rate_si(well_connection)")
    _gas_rate_si         = EclPrototype("double well_conn_get_gas_rate_si(well_connection)")
    _water_rate_si       = EclPrototype("double well_conn_get_water_rate_si(well_connection)")
    _volume_rate_si      = EclPrototype("double well_conn_get_volume_rate_si(well_connection)")

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly")


    def isOpen(self):
        """ @rtype: bool """
        return self._is_open()


    def ijk(self):
        """ @rtype: tuple of (int, int, int) """
        i = self._i()
        j = self._j()
        k = self._k()
        return i, j, k

    def direction(self):
        """ @rtype: WellConnectionDirectionEnum """
        return self._get_dir()

    def segmentId(self):
        """ @rtype: int """
        return self._segment_id()

    def isFractureConnection(self):
        """ @rtype: bool """
        return self._fracture_connection()

    def isMatrixConnection(self):
        """ @rtype: bool """
        return self._matrix_connection()

    def connectionFactor(self):
        """ @rtype: float """
        return self._connection_factor()

    def __eq__(self, other):
        return self._equal(other)

    def __hash__(self):
        return id(self)

    def __ne__(self, other):
        return not self == other

    def free(self):
        pass

    def isMultiSegmentWell(self):
        """ @rtype: bool """
        return self._is_msw()

    def __repr__(self):
        ijk = str(self.ijk())
        frac = 'fracture ' if self.isFractureConnection() else ''
        open_ = 'open ' if self.isOpen() else 'shut '
        msw = ' (multi segment)' if self.isMultiSegmentWell() else ''
        dir = WellConnectionDirectionEnum(self.direction())
        addr = self._address()
        return 'WellConnection(%s %s%s%s, rates = (O:%s,G:%s,W:%s), direction = %s) at 0x%x' % (ijk, frac, open_, msw, self.oilRate(), self.gasRate(), self.waterRate(), dir, addr)

    def gasRate(self):
        return self._gas_rate()

    def waterRate(self):
        return self._water_rate()

    def oilRate(self):
        return self._oil_rate()

    def volumeRate(self):
        return self._volume_rate()

    def gasRateSI(self):
        return self._gas_rate_si()

    def waterRateSI(self):
        return self._water_rate_si()

    def oilRateSI(self):
        return self._oil_rate_si()

    def volumeRateSI(self):
        return self._volume_rate_si()
