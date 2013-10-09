from ert_gui.models.mixins import ModelMixin, AbstractMethodError


class SelectableModelMixin(ModelMixin):
    SELECTION_CHANGED_EVENT = "selection_changed_event" # single element changed

    def registerDefaultEvents(self):
        super(SelectableModelMixin, self).registerDefaultEvents()
        self.observable().addEvent(SelectableModelMixin.SELECTION_CHANGED_EVENT)

    def selectValue(self, value):
        raise AbstractMethodError(self, "selectValue")

    def unselectValue(self, value):
        raise AbstractMethodError(self, "unselectValue")

    def isValueSelected(self, value):
        """ @rtype: bool """
        raise AbstractMethodError(self, "isValueSelected")

    def selectAll(self):
        raise AbstractMethodError(self, "selectAll")

    def unselectAll(self):
        raise AbstractMethodError(self, "unselectAll")



