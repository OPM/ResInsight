from ert_gui.models.mixins import ModelMixin, AbstractMethodError


class ChoiceModelMixin(ModelMixin):
    CHOICE_LIST_CHANGED_EVENT = "choice_list_changed_event"
    CURRENT_CHOICE_CHANGED_EVENT = "current_choice_changed_event"

    def registerDefaultEvents(self):
        super(ChoiceModelMixin, self).registerDefaultEvents()
        self.observable().addEvent(ChoiceModelMixin.CURRENT_CHOICE_CHANGED_EVENT)
        self.observable().addEvent(ChoiceModelMixin.CHOICE_LIST_CHANGED_EVENT)

    def getChoices(self):
        raise AbstractMethodError(self, "getChoices")

    def getCurrentChoice(self):
        raise AbstractMethodError(self, "getCurrentChoice")

    def setCurrentChoice(self, value):
        raise AbstractMethodError(self, "setCurrentChoice")
