from ert_gui.models import ErtConnector
from ert.util import StringList
from ert_gui.models.connectors.init import CaseSelectorModel
from ert_gui.models.connectors.init.case_list import CaseList
from ert_gui.models.mixins import ChoiceModelMixin

""" Returns all initialized cases except the currently selected one. """
class InitializedCaseSelectorModel(ErtConnector, ChoiceModelMixin):

    def __init__(self):
        self.__current_case = None
        self.__initialized_cases = []

        CaseList().observable().attach(CaseList.LIST_CHANGED_EVENT, self.__caseListChanged)
        CaseSelectorModel().observable().attach(CaseSelectorModel.CURRENT_CHOICE_CHANGED_EVENT, self.__currentCaseChanged)
        self.__fetchCases()

        super(InitializedCaseSelectorModel, self).__init__()


    def __fetchCases(self):
        self.__initialized_cases = []

        for case in CaseList().getList():
            # print("%s %s %s" % (current_case, case, self.ert().isCaseInitialized(case)))
            # if case != current_case and self.ert().isCaseInitialized(case):
            if self.ert().isCaseInitialized(case):
                self.__initialized_cases.append(case)

    def __caseListChanged(self):
        self.__fetchCases()

        self.observable().notify(ChoiceModelMixin.CHOICE_LIST_CHANGED_EVENT)
        self.observable().notify(ChoiceModelMixin.CURRENT_CHOICE_CHANGED_EVENT)

    def __currentCaseChanged(self):
        self.observable().notify(ChoiceModelMixin.CHOICE_LIST_CHANGED_EVENT)
        self.observable().notify(ChoiceModelMixin.CURRENT_CHOICE_CHANGED_EVENT)


    def getChoices(self):
        """ @rtype: StringList """
        copy = list(self.__initialized_cases)
        current_case = CaseSelectorModel().getCurrentChoice()

        if current_case in copy:
            copy.remove(current_case)

        return copy

    def getCurrentChoice(self):
        """ @rtype: str """
        case_list = self.getChoices()

        if self.__current_case is None:
            if len(case_list) > 0:
                self.__current_case = case_list[0]
            else:
                self.__current_case = None

        if len(case_list) > 0 and not self.__current_case in case_list:
            self.__current_case = case_list[0]

        return self.__current_case

    def setCurrentChoice(self, case):
        self.__current_case = case
        self.observable().notify(self.CURRENT_CHOICE_CHANGED_EVENT)