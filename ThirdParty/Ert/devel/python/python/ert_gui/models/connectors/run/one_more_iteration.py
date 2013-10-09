from ert_gui.models import ErtConnector
from ert_gui.models.connectors.run import IteratedSmoother, SimulationModeModel
from ert_gui.models.mixins import ButtonModelMixin


class OneMoreIteration(ErtConnector, ButtonModelMixin):

    def __init__(self):
        self.__enabled = False
        SimulationModeModel().observable().attach(SimulationModeModel.CURRENT_CHOICE_CHANGED_EVENT, self.__simulationModeChanged)
        super(OneMoreIteration, self).__init__()

    def __simulationModeChanged(self):
        self.__enabled = SimulationModeModel().getCurrentChoice() == IteratedSmoother()
        self.observable().notify(ButtonModelMixin.BUTTON_STATE_CHANGED_EVENT)

    def buttonTriggered(self):
        print(str(self))

        self.observable().notify(ButtonModelMixin.BUTTON_TRIGGERED_EVENT)

    def getButtonName(self):
        return "Run one more iteration"

    def buttonIsEnabled(self):
        return self.__enabled




