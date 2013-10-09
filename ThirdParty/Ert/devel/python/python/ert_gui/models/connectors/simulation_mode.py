from ert_gui.models import ErtConnector
from ert_gui.models.mixins import ChoiceModelMixin


class SimulationModeModel(ErtConnector, ChoiceModelMixin):
    __modes = ["Ensemble experiment", "EnKF assimilation", "Smoother"]

    def __init__(self):
        self.observable().addEvent(self.CHOICE_LIST_CHANGED_EVENT)
        self.observable().addEvent(self.CURRENT_CHOICE_CHANGED_EVENT)

    def initialize(self):
        self.value = SimulationModeModel.__modes[0]
        self.observable().notify(self.CHOICE_LIST_CHANGED_EVENT)
        self.observable().notify(self.CURRENT_CHOICE_CHANGED_EVENT)

    def getChoices(self):
        return SimulationModeModel.__modes

    def getCurrentChoice(self):
        return self.value

    def setCurrentChoice(self, value):
        self.value = value
        self.observable().notify(self.CURRENT_CHOICE_CHANGED_EVENT)





