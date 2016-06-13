from ert_gui.models import ErtConnector
from ert_gui.models.connectors.run import EnsembleExperiment, EnsembleSmoother, \
     IteratedEnsembleSmoother, MultipleDataAssimilation
from ert_gui.models.mixins import ChoiceModelMixin


class SimulationModeModel(ErtConnector, ChoiceModelMixin):
    __modes = [EnsembleExperiment(), EnsembleSmoother(), IteratedEnsembleSmoother(), MultipleDataAssimilation()]

    def __init__(self):
        self.__value = SimulationModeModel.__modes[0]
        super(SimulationModeModel, self).__init__()

    def getChoices(self):
        return SimulationModeModel.__modes

    def getCurrentChoice(self):
        return self.__value

    def setCurrentChoice(self, value):
        self.__value = value
        self.observable().notify(self.CURRENT_CHOICE_CHANGED_EVENT)
