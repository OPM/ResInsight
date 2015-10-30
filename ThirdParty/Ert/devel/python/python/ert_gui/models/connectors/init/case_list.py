from ert.enkf.enums.realization_state_enum import RealizationStateEnum
from ert_gui.models import ErtConnector
from ert_gui.models.mixins import ListModelMixin
from ert.enkf.state_map import StateMap


class CaseList(ErtConnector, ListModelMixin):

    def getList(self):
        fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        case_list = self.ert().getEnkfFsManager().getCaseList()
        return sorted(case_list)

    def addItem(self, value):
        # Creates a new filesystem. Value should be a case that does not exist
        enkf_fs = self.ert().getEnkfFsManager().getFileSystem(value)
        self.ert().getEnkfFsManager().switchFileSystem(enkf_fs)
        self.observable().notify(ListModelMixin.LIST_CHANGED_EVENT)


    def getAllCasesWithDataAndNotRunning(self):
        cases = self.getList()
        cases_with_data_and_not_running = []
        for case in cases:
            case_has_data = False
            state_map = self.ert().getEnkfFsManager().getStateMapForCase(case)

            for state in state_map:
                if state == RealizationStateEnum.STATE_HAS_DATA:
                    case_has_data = True

            if case_has_data and not self.ert().getEnkfFsManager().isCaseRunning(case):
                cases_with_data_and_not_running.append(case)

        return cases_with_data_and_not_running

    def getAllCasesNotRunning(self):
        cases = self.getList()
        cases_not_running = []
        for case in cases:
            if not self.ert().getEnkfFsManager().isCaseRunning(case):
                cases_not_running.append(case)

        return cases_not_running


    def getCaseRealizationStates(self, case_name):
        state_map = self.ert().getEnkfFsManager().getStateMapForCase(case_name)
        return [state for state in state_map]


    def externalModificationNotification(self):
        self.observable().notify(ListModelMixin.LIST_CHANGED_EVENT)







