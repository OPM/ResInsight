from ert.cwrap import BaseCEnum
from ert.util import UTIL_LIB


class RngAlgTypeEnum(BaseCEnum):
    MZRAN = None


RngAlgTypeEnum.addEnum("MZRAN", 1)
RngAlgTypeEnum.registerEnum(UTIL_LIB, "rng_alg_type_enum")


