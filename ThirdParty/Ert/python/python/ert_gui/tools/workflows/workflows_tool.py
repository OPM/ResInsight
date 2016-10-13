from ert_gui import ERT
from ert_gui.ertwidgets import resourceIcon
from ert_gui.ertwidgets.closabledialog import ClosableDialog
from ert_gui.ertwidgets.models.ertmodel import getWorkflowNames
from ert_gui.tools import Tool
from ert_gui.tools.workflows import RunWorkflowWidget


class WorkflowsTool(Tool):
    def __init__(self):
        enabled = len(getWorkflowNames()) > 0
        super(WorkflowsTool, self).__init__("Run Workflow", "tools/workflows", resourceIcon("ide/to_do_list_checked_1"), enabled)


    def trigger(self):
        run_workflow_widget = RunWorkflowWidget()
        dialog = ClosableDialog("Run workflow", run_workflow_widget, self.parent())
        dialog.exec_()
        ERT.emitErtChange() # workflow may have added new cases.

