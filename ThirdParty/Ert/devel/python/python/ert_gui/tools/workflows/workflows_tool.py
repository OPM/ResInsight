from ert_gui.tools import Tool
from ert_gui.tools.workflows import RunWorkflowWidget
from ert_gui.widgets import util
from ert_gui.widgets.closable_dialog import ClosableDialog


class WorkflowsTool(Tool):
    def __init__(self):
        super(WorkflowsTool, self).__init__("Run Workflow", util.resourceIcon("ide/to_do_list_checked_1"))

    def trigger(self):
        run_workflow_widget = RunWorkflowWidget()
        dialog = ClosableDialog("Run workflow", run_workflow_widget, self.parent())
        dialog.exec_()

