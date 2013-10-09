from ert_gui.models import ErtConnector
from ert_gui.models.mixins import ButtonModelMixin


class EnkfAssimilation(ErtConnector, ButtonModelMixin):

    def buttonTriggered(self):
        print(str(self))

        self.observable().notify(ButtonModelMixin.BUTTON_TRIGGERED_EVENT)

    def getButtonName(self):
        return "Run"

    def buttonIsEnabled(self):
        return False

    def __str__(self):
        return "EnKF Assimilation"


