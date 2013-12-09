from ert.enkf.enums.realization_state_enum import RealizationStateEnum
from ert_gui.models import ErtConnector
from ert_gui.models.mixins import ListModelMixin
from ert.enkf.state_map import StateMap


class CaseList(ErtConnector, ListModelMixin):

    def getList(self):
        fs = self.ert().getEnkfFsManager().getFileSystem()
        case_list = self.ert().getEnkfFsManager().getCaseList()
        return sorted(case_list)

    def addItem(self, value):
        self.ert().getEnkfFsManager().selectFileSystem(value)
        self.observable().notify(ListModelMixin.LIST_CHANGED_EVENT)


    def getAllCasesWithData(self):
        cases = self.getList()
        cases_with_data = []
        for case in cases:
            case_has_data = False
            state_map = self.ert().getEnkfFsManager().getStateMapForCase(case)

            for state in state_map:
                if state == RealizationStateEnum.STATE_HAS_DATA:
                    case_has_data = True

            if case_has_data:
                cases_with_data.append(case)

        return cases_with_data


    def getCaseRealizationStates(self, case_name):
        state_map = self.ert().getEnkfFsManager().getStateMapForCase(case_name)
        return [state for state in state_map]


    def externalModificationNotification(self):
        self.observable().notify(ListModelMixin.LIST_CHANGED_EVENT)







