from ert.cwrap import BaseCEnum
from ert.well import ECL_WELL_LIB

class WellConnectionDirectionEnum(BaseCEnum):
    well_conn_dirX = None
    well_conn_dirY = None
    well_conn_dirZ = None
    well_conn_fracX = None
    well_conn_fracY = None

WellConnectionDirectionEnum.addEnum("well_conn_dirX", 1)
WellConnectionDirectionEnum.addEnum("well_conn_dirY", 2)
WellConnectionDirectionEnum.addEnum("well_conn_dirZ", 3)
WellConnectionDirectionEnum.addEnum("well_conn_fracX", 4)
WellConnectionDirectionEnum.addEnum("well_conn_fracY", 5)

WellConnectionDirectionEnum.registerEnum(ECL_WELL_LIB, "well_connection_dir_enum")