from ert_gui.models import ErtConnector
from ert.util import StringList
from ert_gui.models.connectors.init.case_list import CaseList
from ert_gui.models.mixins import ChoiceModelMixin
from ert_gui.widgets.util import may_take_a_long_time


class CaseSelectorModel(ErtConnector, ChoiceModelMixin):
    def __init__(self):
        CaseList().observable().attach(CaseList.LIST_CHANGED_EVENT, self.__caseListChanged)
        super(CaseSelectorModel, self).__init__()

    def __caseListChanged(self):
        self.observable().notify(ChoiceModelMixin.CHOICE_LIST_CHANGED_EVENT)
        self.observable().notify(ChoiceModelMixin.CURRENT_CHOICE_CHANGED_EVENT)

    def getChoices(self):
        """ @rtype: StringList """
        return CaseList().getList()

    def getCurrentChoice(self):
        """ @rtype: str """
        case_list = self.getChoices()
        current_case = self.ert().getEnkfFsManager().getFileSystem()
        return current_case.getCaseName()

    @may_take_a_long_time
    def setCurrentChoice(self, case):
        case = str(case)
        if not case == "":
            self.ert().getEnkfFsManager().userSelectFileSystem(case)
            self.observable().notify(self.CURRENT_CHOICE_CHANGED_EVENT)