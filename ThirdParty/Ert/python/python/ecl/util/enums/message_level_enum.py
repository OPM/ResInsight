from cwrap import BaseCEnum


class MessageLevelEnum(BaseCEnum):
    TYPE_NAME = "message_level_enum"

    LOG_CRITICAL = None
    LOG_ERROR    = None
    LOG_WARNING  = None
    LOG_INFO     = None
    LOG_DEBUG    = None


MessageLevelEnum.addEnum("LOG_CRITICAL", 0)
MessageLevelEnum.addEnum("LOG_ERROR",    1)
MessageLevelEnum.addEnum("LOG_WARNING",  2)
MessageLevelEnum.addEnum("LOG_INFO",     3)
MessageLevelEnum.addEnum("LOG_DEBUG",    4)
