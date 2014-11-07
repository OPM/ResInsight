from ert.cwrap import BaseCEnum
from ert.well import ECL_WELL_LIB

class WellTypeEnum(BaseCEnum):
    UNDOCUMENTED_ZERO = None
    PRODUCER = None
    WATER_INJECTOR = None
    GAS_INJECTOR = None
    OIL_INJECTOR = None

WellTypeEnum.addEnum("UNDOCUMENTED_ZERO", 0)
WellTypeEnum.addEnum("PRODUCER", 10)
WellTypeEnum.addEnum("WATER_INJECTOR", 22)
WellTypeEnum.addEnum("GAS_INJECTOR", 21)
WellTypeEnum.addEnum("OIL_INJECTOR", 78)

WellTypeEnum.registerEnum(ECL_WELL_LIB, "well_type_enum")