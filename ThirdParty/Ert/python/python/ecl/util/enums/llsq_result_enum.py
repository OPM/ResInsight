from cwrap import BaseCEnum


class LLSQResultEnum(BaseCEnum):
    TYPE_NAME = "llsq_result_enum"
    LLSQ_SUCCESS = None
    LLSQ_INVALID_DIM = None
    LLSQ_UNDETERMINED = None


LLSQResultEnum.addEnum("LLSQ_SUCCESS" , 0)
LLSQResultEnum.addEnum("LLSQ_INVALID_DIM" , 1)
LLSQResultEnum.addEnum("LLSQ_UNDETERMINED" , 2)
