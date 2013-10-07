from ert_gui.models import ErtConnector
from ert.util import StringList
from ert_gui.models.mixins import ChoiceModelMixin
from ert_gui.widgets.util import may_take_a_long_time


class CaseModel(ErtConnector, ChoiceModelMixin):
    def __init__(self):
        self.observable().addEvent(self.CURRENT_CHOICE_CHANGED_EVENT)
        self.observable().addEvent(self.CHOICE_LIST_CHANGED_EVENT)

    def initialize(self):
        self.observable().notify(self.CHOICE_LIST_CHANGED_EVENT)
        self.observable().notify(self.CURRENT_CHOICE_CHANGED_EVENT)

    def getChoices(self):
        """ @rtype: StringList """
        fs = self.ert().getFileSystem()
        case_list = self.ert().getCaseList()
        return case_list

    def getCurrentChoice(self):
        """ @rtype: str """
        case_list = self.getChoices()
        current_case = self.ert().getFileSystem()
        return current_case.getCaseName()

    @may_take_a_long_time
    def setCurrentChoice(self, case):
        case = str(case)
        if not case == "":
            self.ert().userSelectFileSystem(case)
            self.observable().notify(self.CURRENT_CHOICE_CHANGED_EVENT)