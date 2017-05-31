import ecl
import ecl.util
import ecl.geo
import ecl.ecl

from cwrap import Prototype

class WellPrototype(Prototype):
    lib = ecl.load("libecl_well")

    def __init__(self, prototype, bind=True):
        super(WellPrototype, self).__init__(WellPrototype.lib, prototype, bind=bind)

from .well_type_enum import WellTypeEnum
from .well_connection_direction_enum import WellConnectionDirectionEnum
from .well_connection import WellConnection
from .well_segment import WellSegment
from .well_state import WellState
from .well_time_line import WellTimeLine
from .well_info  import WellInfo
