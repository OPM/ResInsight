from ert_gui.models.mixins import ModelMixin, AbstractMethodError


class RangeModelMixin(ModelMixin):
    RANGE_VALUE_CHANGED_EVENT = "range_value_changed_event"

    def registerDefaultEvents(self):
        super(RangeModelMixin, self).registerDefaultEvents()
        self.observable().addEvent(RangeModelMixin.RANGE_VALUE_CHANGED_EVENT)

    def getMaxValue(self):
        raise AbstractMethodError(self, "getMaxValue")

    def getMinValue(self):
        raise AbstractMethodError(self, "getMinValue")

