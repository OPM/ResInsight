from ert.cwrap import BaseCEnum


class UIReturnStatusEnum(BaseCEnum):
    UI_RETURN_OK = None
    UI_RETURN_FAIL = None

UIReturnStatusEnum.addEnum( "UI_RETURN_OK" , 1 )
UIReturnStatusEnum.addEnum( "UI_RETURN_FAIL" , 2 )
