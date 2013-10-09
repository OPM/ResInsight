from ert_gui.models.mixins import ButtonModelMixin


class FunctionButtonModel(ButtonModelMixin):

    def __init__(self, name, function_reference):
        self.__name = name
        self.__function_reference = function_reference
        self.__enabled = True

        super(FunctionButtonModel, self).__init__()

    def getButtonName(self):
        return self.__name

    def buttonTriggered(self):
        self.__function_reference()
        self.observable().notify(ButtonModelMixin.BUTTON_TRIGGERED_EVENT)

    def buttonIsEnabled(self):
        return self.__enabled

    def setEnabled(self, enabled):
        self.__enabled = enabled
        self.observable().notify(ButtonModelMixin.BUTTON_STATE_CHANGED_EVENT)
