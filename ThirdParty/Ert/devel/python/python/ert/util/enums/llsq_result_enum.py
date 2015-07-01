from ert.cwrap import BaseCEnum
from ert.util import UTIL_LIB


class LLSQResultEnum(BaseCEnum):
    LLSQ_SUCCESS = None
    LLSQ_INVALID_DIM = None
    LLSQ_UNDETERMINED = None


LLSQResultEnum.addEnum("LLSQ_SUCCESS" , 0)
LLSQResultEnum.addEnum("LLSQ_INVALID_DIM" , 1)
LLSQResultEnum.addEnum("LLSQ_UNDETERMINED" , 2)

LLSQResultEnum.registerEnum(UTIL_LIB, "llsq_result_enum")


