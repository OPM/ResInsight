from ert.enkf.enums.enkf_state_type_enum import EnkfStateType
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
        source_state = EnkfStateType.ANALYZED
        selected_members = InitializationMembersModel().getSelectedItems()
        total_member_count = EnsembleSizeModel().getSpinnerValue()

        member_mask = BoolVector.createFromList(total_member_count, selected_members)
        ranking_key = None
        selected_parameters = StringList((InitializationParametersModel()).getSelectedItems())

        # print("%s %d %d %s %s" % (source_case, source_report_step, int(source_state), str(selected_members), str(selected_parameters)))

        self.ert().initializeFromExistingCase(source_case, source_report_step, source_state, member_mask, ranking_key, selected_parameters)

        self.observable().notify(ButtonModelMixin.BUTTON_TRIGGERED_EVENT)

    def getButtonName(self):
        return "Initialize"

    def buttonIsEnabled(self):
        return len(InitializedCaseSelectorModel().getChoices()) > 0







