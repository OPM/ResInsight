from ert_gui.models.mixins import ModelMixin, AbstractMethodError


class BooleanModelMixin(ModelMixin):
    BOOLEAN_VALUE_CHANGED_EVENT = "boolean_value_changed_event" # single element changed

    def registerDefaultEvents(self):
        super(BooleanModelMixin, self).registerDefaultEvents()
        self.observable().addEvent(BooleanModelMixin.BOOLEAN_VALUE_CHANGED_EVENT)

    def isTrue(self):
        """ @rtype: bool """
        raise AbstractMethodError(self, "isTrue")

    def setState(self, value):
        raise AbstractMethodError(self, "setTrue")

