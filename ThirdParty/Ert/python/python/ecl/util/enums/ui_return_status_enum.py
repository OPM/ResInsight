from cwrap import BaseCEnum


class UIReturnStatusEnum(BaseCEnum):
    TYPE_NAME = "ui_return_status"
    UI_RETURN_OK = None
    UI_RETURN_FAIL = None


UIReturnStatusEnum.addEnum("UI_RETURN_OK", 1)
UIReturnStatusEnum.addEnum("UI_RETURN_FAIL", 2)
