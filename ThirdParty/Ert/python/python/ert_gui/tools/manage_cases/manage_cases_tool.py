from ert_gui.ertwidgets import resourceIcon
from ert_gui.ertwidgets.closabledialog import ClosableDialog
from ert_gui.tools import Tool
from ert_gui.tools.manage_cases.case_init_configuration import CaseInitializationConfigurationPanel


class ManageCasesTool(Tool):
    def __init__(self):
        super(ManageCasesTool, self).__init__("Manage Cases", "tools/manage_cases", resourceIcon("ide/database_gear"))


    def trigger(self):
        case_management_widget = CaseInitializationConfigurationPanel()

        dialog = ClosableDialog("Manage Cases", case_management_widget, self.parent())
        dialog.exec_()




