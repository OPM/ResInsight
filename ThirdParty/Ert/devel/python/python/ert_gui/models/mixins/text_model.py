from ert_gui.models.mixins import ModelMixin, AbstractMethodError


class TextModelMixin(ModelMixin):
    TEXT_VALUE_CHANGED_EVENT = "text_value_changed_event"

    def registerDefaultEvents(self):
        super(TextModelMixin, self).registerDefaultEvents()
        self.observable().addEvent(TextModelMixin.TEXT_VALUE_CHANGED_EVENT)

    def getText(self):
        """ @rtype: str """
        raise AbstractMethodError(self, "getText")

    def setText(self, value):
        raise AbstractMethodError(self, "setText")

