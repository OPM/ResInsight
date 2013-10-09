from ert_gui.models.mixins import ModelMixin, AbstractMethodError


class ListModelMixin(ModelMixin):
    LIST_CHANGED_EVENT = "list_value_changed_event"

    def registerDefaultEvents(self):
        super(ListModelMixin, self).registerDefaultEvents()
        self.observable().addEvent(ListModelMixin.LIST_CHANGED_EVENT)

    def getList(self):
        """ @rtype: list """
        raise AbstractMethodError(self, "getList")

    def addItem(self, value):
        raise AbstractMethodError(self, "addItem")

    def removeItem(self, value):
        raise AbstractMethodError(self, "removeItem")
