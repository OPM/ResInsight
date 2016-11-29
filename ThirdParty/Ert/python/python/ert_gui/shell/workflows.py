from ert.job_queue import WorkflowRunner
from ert_gui.shell import assertConfigLoaded, ErtShellCollection
from ert_gui.shell.libshell import autoCompleteList


class Workflows(ErtShellCollection):
    def __init__(self, parent):
        super(Workflows, self).__init__("workflows", parent)

        self.addShellFunction(name="list",
                              function=Workflows.list,
                              help_message="Shows a list of all available workflows.")

        self.addShellFunction(name="run",
                              function=Workflows.run,
                              completer=Workflows.completeRun,
                              help_arguments="<workflow_name>",
                              help_message="Run a named workflow.")

    def getWorkflowNames(self):
        return [workflow for workflow in self.ert().getWorkflowList().getWorkflowNames()]

    @assertConfigLoaded
    def list(self, line):
        workflows = self.getWorkflowNames()
        if len(workflows) > 0:
            self.columnize(workflows)
        else:
            print("No workflows available.")

    @assertConfigLoaded
    def run(self, workflow):
        workflow = workflow.strip()
        if workflow in self.getWorkflowNames():
            workflow_list = self.ert().getWorkflowList()
            workflow = workflow_list[workflow]
            context = workflow_list.getContext()

            runner = WorkflowRunner(workflow, self.ert(), context)
            runner.run()
            runner.wait()
        else:
            self.lastCommandFailed("Unknown workflow: '%s'" % workflow)

    @assertConfigLoaded
    def completeRun(self, text, line, begidx, endidx):
        return autoCompleteList(text, self.getWorkflowNames())

