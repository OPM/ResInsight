from cwrap import BaseCEnum
from ert.well import ECL_WELL_LIB

class WellTypeEnum(BaseCEnum):
    ERT_UNDOCUMENTED_ZERO = None
    ERT_PRODUCER = None
    ERT_WATER_INJECTOR = None
    ERT_GAS_INJECTOR = None
    ERT_OIL_INJECTOR = None

WellTypeEnum.addEnum("ERT_UNDOCUMENTED_ZERO", 0)
WellTypeEnum.addEnum("ERT_PRODUCER", 10)
WellTypeEnum.addEnum("ERT_WATER_INJECTOR", 22)
WellTypeEnum.addEnum("ERT_GAS_INJECTOR", 21)
WellTypeEnum.addEnum("ERT_OIL_INJECTOR", 78)

WellTypeEnum.registerEnum(ECL_WELL_LIB, "well_type_enum")