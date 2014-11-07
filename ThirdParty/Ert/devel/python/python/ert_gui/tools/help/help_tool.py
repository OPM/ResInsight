from ert_gui.tools import Tool
from ert_gui.tools.help import HelpWindow
from ert_gui.widgets import util


class HelpTool(Tool):
    __help_window_instance = None

    def __init__(self, help_center_name, parent):
        super(HelpTool, self).__init__("Help", "tools/help", util.resourceIcon("ide/help"), enabled=True, checkable=True)
        self.setParent(parent)

        self.help_center_name = help_center_name

        if HelpTool.__help_window_instance is None:
            HelpTool.__help_window_instance = HelpWindow(self.help_center_name, parent=self.parent())
            self.__help_window_instance.visibilityChanged.connect(self.getAction().setChecked)


    def trigger(self):
        checked = self.getAction().isChecked()
        if checked:
            HelpTool.__help_window_instance.show()
        else:
            HelpTool.__help_window_instance.hide()

