from ert_gui.models import ErtConnector
from ert_gui.models.mixins import BasicModelMixin


class LsfRequest(ErtConnector, BasicModelMixin):

    def getValue(self):
        """ @rtype: str """
        return self.ert().siteConfig().getLsfRequest()

    def setValue(self, value):
        self.ert().siteConfig().setLsfRequest(value)



