from ert_gui.models.mixins import ChoiceModelMixin, ListModelMixin


class DefaultChoiceListModel(ListModelMixin, ChoiceModelMixin):

    def __init__(self, choices, default_choice=None):
        assert choices is not None and len(choices) > 0
        self.__choices = choices
        self.__value = None if not default_choice in choices else default_choice
        super(DefaultChoiceListModel, self).__init__()

    def getList(self):
        return self.__choices

    def getChoices(self):
        return self.getList()

    def getCurrentChoice(self):
        if self.__value is None:
            return self.getList()[0]
        return self.__value

    def setCurrentChoice(self, value):
        self.__value = value
        self.observable().notify(self.CURRENT_CHOICE_CHANGED_EVENT)

