from ert_gui.ertwidgets import resourceIcon
from ert_gui.tools import Tool
from ert_gui.tools.plot import PlotWindow


class PlotTool(Tool):
    def __init__(self):
        super(PlotTool, self).__init__("Create Plot", "tools/plot", resourceIcon("ide/chart_curve_add"))

    def trigger(self):
        plot_window = PlotWindow(self.parent())
        plot_window.show()


