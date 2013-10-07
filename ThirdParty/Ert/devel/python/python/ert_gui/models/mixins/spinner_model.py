from ert_gui.models.mixins import RangeModelMixin, AbstractMethodError


class SpinnerModelMixin(RangeModelMixin):
    SPINNER_VALUE_CHANGED_EVENT = "spinner_value_changed_event"

    def registerDefaultEvents(self):
        super(SpinnerModelMixin, self).registerDefaultEvents()
        self.observable().addEvent(SpinnerModelMixin.SPINNER_VALUE_CHANGED_EVENT)

    def getSpinnerValue(self):
        raise AbstractMethodError(self, "getSpinnerValue")

    def setSpinnerValue(self, value):
        raise AbstractMethodError(self, "setSpinnerValue")
