from ert_gui.tools.manage_cases.case_init_configuration import CaseInitializationConfigurationPanel
from ert_gui.tools import Tool
from ert_gui.widgets import util
from ert_gui.widgets.closable_dialog import ClosableDialog


class ManageCasesTool(Tool):
    def __init__(self):
        super(ManageCasesTool, self).__init__("Manage Cases", "tools/manage_cases", util.resourceIcon("ide/database_gear"))


    def trigger(self):
        case_management_widget = CaseInitializationConfigurationPanel()

        dialog = ClosableDialog("Manage Cases", case_management_widget, self.parent())
        dialog.exec_()




