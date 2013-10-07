from ert_gui.models.mixins import ModelMixin


class KeywordModelMixin(ModelMixin):
    KEYWORD_VALUE_CHANGED_EVENT = "keyword_value_changed_event"

    def getKeyName(self):
        """ @rtype: str """
        raise NotImplementedError("Class %s has not implemented support for %s()" % (self.__class__.__name__, "getKeyName"))

    def getValueName(self):
        """ @rtype: str """
        raise NotImplementedError("Class %s has not implemented support for %s()" % (self.__class__.__name__, "getValueName"))

    def getDefaultValue(self):
        """ @rtype: str """
        raise NotImplementedError("Class %s has not implemented support for %s()" % (self.__class__.__name__, "getDefaultValue"))

    def getKeywords(self):
        """ @rtype: dict """
        raise NotImplementedError("Class %s has not implemented support for %s()" % (self.__class__.__name__, "getKeywords"))

    def setKeywords(self, value):
        raise NotImplementedError("Class %s has not implemented support for %s()" % (self.__class__.__name__, "setKeywords"))
