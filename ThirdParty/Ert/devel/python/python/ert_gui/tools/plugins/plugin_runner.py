from functools import partial
from threading import Thread
import time
from ert.job_queue.ert_plugin import CancelPluginException
from ert_gui.tools.plugins import Plugin, ProcessJobDialog
from ert.job_queue import WorkflowJob

class PluginRunner(object):
    def __init__(self, plugin):
        """
         @type plugin: Plugin
        """
        super(PluginRunner, self).__init__()

        self.__plugin = plugin

        self.__plugin_finished_callback = lambda : None

        self.__result = None

    def run(self):
        try:
            plugin = self.__plugin

            arguments = plugin.getArguments()
            dialog = ProcessJobDialog(plugin.getName(), plugin.getParentWindow())

            dialog.cancelConfirmed.connect(self.cancel)

            run_function = partial(self.__runWorkflowJob, plugin, arguments)

            workflow_job_thread = Thread(name="ert_gui_workflow_job_thread")
            workflow_job_thread.setDaemon(True)
            workflow_job_thread.run = run_function
            workflow_job_thread.start()


            poll_function = partial(self.__pollRunner, plugin, dialog)

            poll_thread = Thread(name="ert_gui_workflow_job_poll_thread")
            poll_thread.setDaemon(True)
            poll_thread.run = poll_function
            poll_thread.start()

            dialog.show()
        except CancelPluginException:
            print("Plugin cancelled before execution!")

    def __runWorkflowJob(self, plugin, arguments):
        workflow_job = plugin.getWorkflowJob()
        self.__result = workflow_job.run(plugin.ert(), arguments)


    def __pollRunner(self, plugin, dialog):
        self.wait()

        details = ""
        if self.__result is not None:
            details = str(self.__result)

        if plugin.getWorkflowJob().hasFailed():
            dialog.presentError.emit("Job Failed!", "The job '%s' has failed while running!" % plugin.getName(), details)
            dialog.disposeDialog.emit()
        elif plugin.getWorkflowJob().isCancelled():
            dialog.presentInformation.emit("Job Cancelled!", "The job '%s' was cancelled successfully!" % plugin.getName(), details)
            dialog.disposeDialog.emit()
        else:
            dialog.presentInformation.emit("Job Completed!", "The job '%s' was completed successfully!" % plugin.getName(), details)
            dialog.disposeDialog.emit()

        self.__plugin_finished_callback()


    def isRunning(self):
        """ @rtype: bool """
        return self.__plugin.getWorkflowJob().isRunning()

    def isCancelled(self):
        """ @rtype: bool """
        return self.__plugin.getWorkflowJob().isCancelled()

    def cancel(self):
        if self.isRunning():
            self.__plugin.getWorkflowJob().cancel()

    def wait(self):
        while self.isRunning():
            time.sleep(1)

    def setPluginFinishedCallback(self, callback):
        self.__plugin_finished_callback = callback