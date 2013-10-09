from ert_gui.models import ErtConnector
from ert_gui.models.mixins import BasicModelMixin


class LsfQueue(ErtConnector, BasicModelMixin):

    def getValue(self):
        """ @rtype: str """
        return self.ert().siteConfig().getLsfQueue()

    def setValue(self, value):
        self.ert().siteConfig().setLsfQueue(value)



