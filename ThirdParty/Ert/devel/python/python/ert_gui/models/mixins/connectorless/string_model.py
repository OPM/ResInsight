from ert_gui.models.mixins import BasicModelMixin


class StringModel(BasicModelMixin):

    def __init__(self, value = ""):
        self.__value = value
        super(StringModel , self).__init__()

    def setValue(self, value):
        self.__value = value
        self.observable().notify(self.VALUE_CHANGED_EVENT)

    def getValue(self):
        """ @rtype: str """
        return self.__value




