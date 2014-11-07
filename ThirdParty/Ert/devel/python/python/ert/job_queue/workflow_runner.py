from threading import Thread
from ert.job_queue import Workflow
from ert.util.substitution_list import SubstitutionList


class WorkflowRunner(object):
    def __init__(self, workflow, ert=None, context=None):
        """
         @type workflow: Workflow
         @type ert: ert.enkf.EnKFMain
         @type context: SubstitutionList
        """
        super(WorkflowRunner, self).__init__()

        self.__workflow = workflow
        self.__ert = ert

        if context is None:
            context = SubstitutionList()

        self.__context = context
        self.__workflow_result = None

    def run(self):
        workflow_thread = Thread(name="ert_gui_workflow_thread")
        workflow_thread.setDaemon(True)
        workflow_thread.run = self.__runWorkflow
        workflow_thread.start()

    def __runWorkflow(self):
        self.__workflow_result = self.__workflow.run(self.__ert, context=self.__context)

    def isRunning(self):
        """ @rtype: bool """
        return self.__workflow.isRunning()

    def isCancelled(self):
        """ @rtype: bool """
        return self.__workflow.isCancelled()

    def cancel(self):
        if self.isRunning():
            self.__workflow.cancel()

    def wait(self):
        self.__workflow.wait()

    def workflowResult(self):
        """ @rtype: bool or None """
        return self.__workflow_result

    def workflowError(self):
        """ @rtype: str """
        error = self.__workflow.getLastError()

        error_message = ""

        for error_line in error:
            error_message += error_line + "\n"

        return error_message


