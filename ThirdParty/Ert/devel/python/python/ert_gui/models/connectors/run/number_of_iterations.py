from ert_gui.models import ErtConnector
from ert_gui.models.mixins import SpinnerModelMixin, BasicModelMixin


class NumberOfIterationsModel(ErtConnector, SpinnerModelMixin, BasicModelMixin):


    def getMaxValue(self):
        """ @rtype: int """
        return 100

    def getMinValue(self):
        """ @rtype: int """
        return 1

    def getSpinnerValue(self):
        """ @rtype: int """
        return self.ert().analysisConfig().getAnalysisIterConfig().getNumIterations()

    def setSpinnerValue(self, value):
        self.ert().analysisConfig().getAnalysisIterConfig().setNumIterations(value)
        self.observable().notify(self.SPINNER_VALUE_CHANGED_EVENT)
        self.observable().notify(self.VALUE_CHANGED_EVENT)

    def getValue(self):
        return self.getSpinnerValue()

    def setValue(self, value):
        self.setSpinnerValue(value)





