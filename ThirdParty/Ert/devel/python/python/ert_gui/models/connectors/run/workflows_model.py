from ert_gui.models import ErtConnector
from ert_gui.models.mixins import ChoiceModelMixin
from ert_gui.models.mixins.list_model import ListModelMixin

from ert.job_queue import WorkflowRunner, Workflow


class WorkflowsModel(ErtConnector, ListModelMixin, ChoiceModelMixin):

    def __init__(self):
        self.__value = None
        super(WorkflowsModel, self).__init__()

    def getList(self):
        return sorted(self.ert().getWorkflowList().getWorkflowNames(), key=lambda v: v.lower())

    def getChoices(self):
        return self.getList()

    def getCurrentChoice(self):
        if self.__value is None:
            return self.getList()[0]
        return self.__value

    def setCurrentChoice(self, value):
        self.__value = value
        self.observable().notify(self.CURRENT_CHOICE_CHANGED_EVENT)


    def createWorkflowRunner(self):
        """ @rtype: WorkflowRunner """
        workflow_name = self.getCurrentChoice()
        workflow_list = self.ert().getWorkflowList()

        workflow = workflow_list[workflow_name]
        context = workflow_list.getContext()
        return WorkflowRunner(workflow, self.ert(), context)
