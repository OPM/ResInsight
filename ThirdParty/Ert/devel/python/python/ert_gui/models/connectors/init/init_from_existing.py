from ert.util import StringList, BoolVector
from ert_gui.models import ErtConnector
from ert_gui.models.connectors.ensemble_resizer import EnsembleSizeModel
from ert_gui.models.connectors.init import InitializationParametersModel, InitializationMembersModel
from ert_gui.models.connectors.init.init_history_length import HistoryLengthModel
from ert_gui.models.connectors.init.initialized_case_selector import InitializedCaseSelectorModel
from ert_gui.models.mixins import ButtonModelMixin


class InitializeFromExistingCaseModel(ErtConnector, ButtonModelMixin):

    def __init__(self):
        InitializedCaseSelectorModel().observable().attach(InitializedCaseSelectorModel.CHOICE_LIST_CHANGED_EVENT, self.__caseChanged)
        super(InitializeFromExistingCaseModel, self).__init__()

    def __caseChanged(self):
        self.observable().notify(ButtonModelMixin.BUTTON_STATE_CHANGED_EVENT)

    def buttonTriggered(self):
        source_case = InitializedCaseSelectorModel().getCurrentChoice()
        source_report_step = HistoryLengthModel().getSpinnerValue()
        selected_members = InitializationMembersModel().getSelectedItems()
        total_member_count = EnsembleSizeModel().getSpinnerValue()

        member_mask = BoolVector.createFromList(total_member_count, selected_members)
        selected_parameters = StringList((InitializationParametersModel()).getSelectedItems())

        self.ert().getEnkfFsManager().customInitializeCurrentFromExistingCase(source_case, source_report_step, member_mask, selected_parameters)

        self.observable().notify(ButtonModelMixin.BUTTON_TRIGGERED_EVENT)

    def getButtonName(self):
        return "Initialize"

    def buttonIsEnabled(self):
        return len(InitializedCaseSelectorModel().getChoices()) > 0







