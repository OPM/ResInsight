from ert.cwrap import BaseCEnum


class ErrorAction(BaseCEnum):
    TYPE_NAME = "error_action_enum"
    THROW_EXCEPTION = None
    WARN = None
    IGNORE = None


ErrorAction.addEnum("THROW_EXCEPTION", 0)
ErrorAction.addEnum("WARN", 1)
ErrorAction.addEnum("IGNORE", 2)
