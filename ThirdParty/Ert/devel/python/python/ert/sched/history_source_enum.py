from ert.cwrap import BaseCEnum
from ert.sched import SCHED_LIB


class HistorySourceEnum(BaseCEnum):
    pass

HistorySourceEnum.addEnum("SCHEDULE", 0)
HistorySourceEnum.addEnum("REFCASE_SIMULATED", 1)
HistorySourceEnum.addEnum("REFCASE_HISTORY", 2)
HistorySourceEnum.addEnum("HISTORY_SOURCE_INVALID", 10)

HistorySourceEnum.registerEnum(SCHED_LIB, "history_source_enum")
