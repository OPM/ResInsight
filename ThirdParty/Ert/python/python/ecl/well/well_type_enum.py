from cwrap import BaseCEnum

class WellTypeEnum(BaseCEnum):
    TYPE_NAME = "well_type_enum"
    ECL_WELL_ZERO = None
    ECL_WELL_PRODUCER = None
    ECL_WELL_WATER_INJECTOR = None
    ECL_WELL_GAS_INJECTOR = None
    ECL_WELL_OIL_INJECTOR = None

WellTypeEnum.addEnum("ECL_WELL_ZERO", 0)
WellTypeEnum.addEnum("ECL_WELL_PRODUCER", 1)
WellTypeEnum.addEnum("ECL_WELL_OIL_INJECTOR", 2)
WellTypeEnum.addEnum("ECL_WELL_WATER_INJECTOR", 3)
WellTypeEnum.addEnum("ECL_WELL_GAS_INJECTOR", 4)

