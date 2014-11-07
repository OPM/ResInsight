import os
from ert_gui.models import ErtConnector
from ert_gui.models.connectors.init.case_selector import CaseSelectorModel


class PlotSettingsModel(ErtConnector):

    def getDefaultPlotPath(self):
        """ @rtype: str """
        path = self.ert().plotConfig().getPath()
        if not os.path.exists(path):
            os.makedirs(path)
        return path


