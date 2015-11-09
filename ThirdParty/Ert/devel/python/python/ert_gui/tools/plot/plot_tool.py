from ert_gui.tools import Tool
from ert_gui.tools.plot import PlotWindow
from ert_gui.widgets import util


class PlotTool(Tool):
    def __init__(self, ert):
        super(PlotTool, self).__init__("Create Plot", "tools/plot", util.resourceIcon("ide/chart_curve_add"))
        self.__ert = ert

    def trigger(self):
        plot_window = PlotWindow(self.__ert, self.parent())
        plot_window.show()


