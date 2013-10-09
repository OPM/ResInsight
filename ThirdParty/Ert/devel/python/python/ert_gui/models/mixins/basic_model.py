from ert_gui.models.mixins import ModelMixin, AbstractMethodError


class BasicModelMixin(ModelMixin):
    VALUE_CHANGED_EVENT = "value_changed_event"

    def registerDefaultEvents(self):
        super(BasicModelMixin, self).registerDefaultEvents()
        self.observable().addEvent(BasicModelMixin.VALUE_CHANGED_EVENT)

    def getValue(self):
        raise AbstractMethodError(self, "getValue")

    def setValue(self, value):
        raise AbstractMethodError(self, "setValue")

