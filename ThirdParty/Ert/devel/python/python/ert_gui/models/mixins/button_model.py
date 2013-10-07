from ert_gui.models.mixins import ModelMixin, AbstractMethodError


class ButtonModelMixin(ModelMixin):
    BUTTON_TRIGGERED_EVENT = "button_triggered_event"
    BUTTON_STATE_CHANGED_EVENT = "button_state_changed_event"

    def registerDefaultEvents(self):
        super(ButtonModelMixin, self).registerDefaultEvents()
        self.observable().addEvent(ButtonModelMixin.BUTTON_TRIGGERED_EVENT)
        self.observable().addEvent(ButtonModelMixin.BUTTON_STATE_CHANGED_EVENT)

    def getButtonName(self):
        raise AbstractMethodError(self, "getButtonName")

    def buttonTriggered(self):
        raise AbstractMethodError(self, "buttonTriggered")

    def buttonIsEnabled(self):
        """ @rtype: bool """
        raise AbstractMethodError(self, "buttonIsEnabled")


