from weakref import ref
from ert_gui.tools import Tool
from ert_gui.tools.ide import IdeWindow
from ert_gui.widgets import util


class IdeTool(Tool):
    def __init__(self, path, reload_function, help_tool):
        super(IdeTool, self).__init__("Configure", "tools/ide", util.resourceIcon("ide/widgets"))

        self.ide_window = None
        self.path = path
        self.reload_function = reload_function
        self.help_tool = help_tool

    def trigger(self):
        if self.ide_window is None:
            self.ide_window = ref(IdeWindow(self.path, self.parent(), self.help_tool))
            self.ide_window().reloadTriggered.connect(self.reload_function)

        self.ide_window().show()
        self.ide_window().raise_()
        self.ide_window().activateWindow()