import ert
import ert.util
import ert.geo
import ert.ecl

ECL_WELL_LIB = ert.load("libecl_well")


from .well_type_enum import WellTypeEnum
from .well_connection_direction_enum import WellConnectionDirectionEnum
from .well_connection import WellConnection
from .well_segment import WellSegment
from .well_state import WellState
from .well_time_line import WellTimeLine
from .well_info  import WellInfo
