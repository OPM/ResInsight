from ert_gui.models import ErtConnector
from ert_gui.models.mixins import ButtonModelMixin


class IteratedSmoother(ErtConnector, ButtonModelMixin):

    def buttonTriggered(self):
        print(str(self))

        self.observable().notify(ButtonModelMixin.BUTTON_TRIGGERED_EVENT)

    def getButtonName(self):
        return "Run"

    def __str__(self):
        return "Iterated Smoother"

    def buttonIsEnabled(self):
        return False



