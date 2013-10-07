from ert_gui.models.ert_connector import ErtConnector
from ert_gui.models.mixins import SpinnerModelMixin


class HistoryLengthModel(ErtConnector, SpinnerModelMixin):

    def __init__(self):
        self.__value = self.getMaxValue()
        super(HistoryLengthModel, self).__init__()

        #todo: notify when the history length changes...

    def getMinValue(self):
        """ @rtype: int """
        return 0

    def getMaxValue(self):
        """ @rtype: int """
        return self.ert().getHistoryLength()

    def getSpinnerValue(self):
        return self.__value

    def setSpinnerValue(self, value):
        self.__value = value
        self.observable().notify(SpinnerModelMixin.SPINNER_VALUE_CHANGED_EVENT)

    def setToMax(self):
        self.setSpinnerValue(self.getMaxValue())

    def setToMin(self):
        self.setSpinnerValue(self.getMinValue())



