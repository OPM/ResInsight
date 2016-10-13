from weakref import ref

from ert_gui import ERT
from ert_gui.ertwidgets import resourceIcon
from ert_gui.tools import Tool
from ert_gui.tools.ide import IdeWindow


class IdeTool(Tool):
    def __init__(self, path, help_tool):
        super(IdeTool, self).__init__("Configure", "tools/ide", resourceIcon("ide/widgets"))

        self.ide_window = None
        self.path = path
        self.help_tool = help_tool

    def trigger(self):
        if self.ide_window is None:
            self.ide_window = ref(IdeWindow(self.path, self.parent(), self.help_tool))
            self.ide_window().reloadTriggered.connect(ERT.reloadERT)

        self.ide_window().show()
        self.ide_window().raise_()
        self.ide_window().activateWindow()