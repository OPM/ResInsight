from ert.util import StringList
from ert_gui.models import ErtConnector
from ert_gui.models.connectors.init import InitializationParametersModel, InitializationMembersModel
from ert_gui.models.mixins import ButtonModelMixin


class InitializeFromScratchModel(ErtConnector, ButtonModelMixin):

    def buttonTriggered(self):
        selected_parameters = StringList((InitializationParametersModel()).getSelectedItems())
        members = (InitializationMembersModel()).getSelectedItems()

        for member in members:
            member = int(member.strip())
            self.ert().initializeFromScratch(selected_parameters, member, member)

        self.observable().notify(ButtonModelMixin.BUTTON_TRIGGERED_EVENT)

    def getButtonName(self):
        return "Initialize"

    def buttonIsEnabled(self):
        return True


