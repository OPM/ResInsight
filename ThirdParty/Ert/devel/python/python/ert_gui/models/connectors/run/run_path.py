from ert_gui.models import ErtConnector
from ert.enkf import ModelConfig
from ert_gui.models.mixins import BasicModelMixin


class RunPathModel(ErtConnector, BasicModelMixin):
    def getValue(self):
        """ @rtype: str """
        return self.getModelConfig().getRunpathAsString()

    def setValue(self, run_path):
        self.getModelConfig().setRunpath(str(run_path))
        self.observable().notify(self.VALUE_CHANGED_EVENT)

    def getModelConfig(self):
        """ @rtype: ModelConfig """
        return self.ert().getModelConfig()







