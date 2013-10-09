from ert_gui.models.mixins import ModelMixin, AbstractMethodError


class DictionaryModelMixin(ModelMixin):
    DICTIONARY_CHANGED_EVENT = "dictionary_value_changed_event"

    def registerDefaultEvents(self):
        super(DictionaryModelMixin, self).registerDefaultEvents()
        self.observable().addEvent(DictionaryModelMixin.DICTIONARY_CHANGED_EVENT)

    def getDictionary(self):
        """ @rtype: dict """
        raise AbstractMethodError(self, "getDictionary")

    def addKey(self, key):
        raise AbstractMethodError(self, "addKey")

    def setValueForKey(self, key, value):
        raise AbstractMethodError(self, "setValueForKey")

    def removeKey(self, key):
        raise AbstractMethodError(self, "removeKey")