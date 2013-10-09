from ert_gui.models import ErtConnector
from ert_gui.models.mixins import SpinnerModelMixin


class RshMaxRunning(ErtConnector, SpinnerModelMixin):

    def getMaxValue(self):
        """ @rtype: int """
        return 1000

    def getMinValue(self):
        """ @rtype: int """
        return 1

    def getSpinnerValue(self):
        """ @rtype: int """
        return self.ert().siteConfig().getMaxRunningRsh()

    def setSpinnerValue(self, value):
        self.ert().siteConfig().setMaxRunningRsh(value)



