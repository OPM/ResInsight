from ert.cwrap import BaseCEnum
from ert.util import UTIL_LIB


class RngInitModeEnum(BaseCEnum):
    INIT_DEFAULT        = None
    INIT_CLOCK          = None
    INIT_DEV_RANDOM     = None
    INIT_DEV_URANDOM    = None


RngInitModeEnum.addEnum("INIT_DEFAULT", 0)
RngInitModeEnum.addEnum("INIT_CLOCK", 1)
RngInitModeEnum.addEnum("INIT_DEV_RANDOM", 2)
RngInitModeEnum.addEnum("INIT_DEV_URANDOM", 3)

RngInitModeEnum.registerEnum(UTIL_LIB, "rng_init_mode_enum")

