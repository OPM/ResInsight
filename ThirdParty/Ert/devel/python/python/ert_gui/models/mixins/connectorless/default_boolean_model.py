from ert_gui.models.mixins import BooleanModelMixin


class DefaultBooleanModel(BooleanModelMixin):

    def __init__(self, state=True):
        self.__state = state
        super(DefaultBooleanModel, self).__init__()

    def setState(self, value):
        self.__state = value
        self.observable().notify(self.BOOLEAN_VALUE_CHANGED_EVENT)

    def isTrue(self):
        """ @rtype: bool """
        return self.__state




