from ert.job_queue import WorkflowRunner
from ert_gui.shell import ShellFunction, assertConfigLoaded, autoCompleteList


class Workflows(ShellFunction):
    def __init__(self, shell_context):
        super(Workflows, self).__init__("workflows", shell_context)

        self.addHelpFunction("list", None, "Shows a list of all available workflows.")
        self.addHelpFunction("run", "<workflow_name>", "Run a named workflow.")

    def getWorkflowNames(self):
        return [workflow for workflow in self.ert().getWorkflowList().getWorkflowNames()]

    @assertConfigLoaded
    def do_list(self, line):
        workflows = self.getWorkflowNames()
        if len(workflows) > 0:
            self.columnize(workflows)
        else:
            print("No workflows available.")

    @assertConfigLoaded
    def do_run(self, workflow):
        workflow = workflow.strip()
        if workflow in self.getWorkflowNames():
            workflow_list = self.ert().getWorkflowList()
            workflow = workflow_list[workflow]
            context = workflow_list.getContext()

            runner = WorkflowRunner(workflow, self.ert(), context)
            runner.run()
            runner.wait()
        else:
            print("Error: Unknown workflow: '%s'" % workflow)

    @assertConfigLoaded
    def complete_run(self, text, line, begidx, endidx):
        return autoCompleteList(text, self.getWorkflowNames())