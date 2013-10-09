from ert_gui.models import ErtConnector
from ert_gui.models.mixins import SpinnerModelMixin


class EnsembleSizeModel(ErtConnector, SpinnerModelMixin):

    def getMaxValue(self):
        """ @rtype: int """
        return 10000

    def getMinValue(self):
        """ @rtype: int """
        return 1

    def getSpinnerValue(self):
        """ @rtype: int """
        return self.ert().getEnsembleSize()

    def setSpinnerValue(self, value):
        self.ert().resizeEnsemble(int(value))
        self.observable().notify(self.SPINNER_VALUE_CHANGED_EVENT)



