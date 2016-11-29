from cwrap import BaseCClass
from ert.util import CTime
from opm import OPMPrototype

from opm.parser import ParseContext
from opm.ecl_state.grid import EclipseGrid

class Schedule(BaseCClass):
    TYPE_NAME = "schedule"
    _alloc = OPMPrototype("void* schedule_alloc( parse_context , eclipse_grid , deck )" , bind = False)
    _free  = OPMPrototype("void  schedule_free( schedule )")
    _end_time = OPMPrototype("time_t schedule_end(schedule)")
    _start_time = OPMPrototype("time_t schedule_start(schedule)")
    
    
    def __init__(self , grid, deck , parse_context = None):
        if parse_context is None:
            parse_context = ParseContext( )
        c_ptr = self._alloc( parse_context , grid , deck )
        super(Schedule , self).__init__( c_ptr )


    def endTime(self):
        ct = self._end_time( )
        return ct.datetime( )


    def startTime(self):
        ct = self._start_time( )
        return ct.datetime( )
    

    def free(self):
        self._free( )

