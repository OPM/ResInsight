from PyQt4.QtCore import Qt
from PyQt4.QtGui import QMainWindow, QDockWidget, QTabWidget
from ert_gui.models.connectors.init.case_selector import CaseSelectorModel
from ert_gui.models.connectors.plot import EnsembleSummaryPlot
from ert_gui.tools.plot import PlotPanel
from ert_gui.tools.plot import DataTypeKeysWidget
from ert_gui.tools.plot.plot_case_selection_widget import CaseSelectionWidget
from ert_gui.widgets.util import may_take_a_long_time


class PlotWindow(QMainWindow):
    def __init__(self, parent):
        QMainWindow.__init__(self, parent)

        self.setMinimumWidth(750)
        self.setMinimumHeight(500)

        self.setWindowTitle("Plotting")
        self.activateWindow()

        self.central_tab = QTabWidget()
        self.setCentralWidget(self.central_tab)

        self.plot_panel = PlotPanel("Plot", "gui/plots/simple_plot.html")
        self.plot_panel.plotReady.connect(self.plotReady)
        self.central_tab.addTab(self.plot_panel, "Ensemble plot")

        self.plot_overview_panel = PlotPanel("Plot", "gui/plots/simple_overview_plot.html")
        self.plot_overview_panel.plotReady.connect(self.plotReady)
        self.central_tab.addTab(self.plot_overview_panel, "Ensemble overview plot")

        self.plot_debug_panel = PlotPanel("Debug", "gui/plots/simple_debug_plot.html")
        self.plot_debug_panel.plotReady.connect(self.plotReady)
        self.central_tab.addTab(self.plot_debug_panel, "Debug")

        self.data_type_keys_widget = DataTypeKeysWidget()
        self.data_type_keys_widget.dataTypeKeySelected.connect(self.keySelected)
        self.addDock("Data types", self.data_type_keys_widget)


        current_case = CaseSelectorModel().getCurrentChoice()
        self.case_selection_widget = CaseSelectionWidget(current_case)
        self.case_selection_widget.caseSelectionChanged.connect(self.caseSelectionChanged)
        self.addDock("Plot case", self.case_selection_widget)

        self.__data_type_key = None
        self.__plot_cases = self.case_selection_widget.getPlotCaseNames()






    def addDock(self, name, widget, area=Qt.LeftDockWidgetArea, allowed_areas=Qt.AllDockWidgetAreas):
        dock_widget = QDockWidget(name)
        dock_widget.setObjectName("%sDock" % name)
        dock_widget.setWidget(widget)
        dock_widget.setAllowedAreas(allowed_areas)
        dock_widget.setFeatures(QDockWidget.DockWidgetFloatable | QDockWidget.DockWidgetMovable)

        self.addDockWidget(area, dock_widget)
        return dock_widget


    def checkPlotStatus(self):
        return self.plot_panel.isReady() and self.plot_debug_panel.isReady() and self.plot_overview_panel.isReady()

    def plotReady(self):
        if self.checkPlotStatus():
            self.data_type_keys_widget.selectDefault()


    def caseSelectionChanged(self):
        self.__plot_cases = self.case_selection_widget.getPlotCaseNames()
        self.keySelected(self.__data_type_key)

    @may_take_a_long_time
    def keySelected(self, key):
        self.__data_type_key = str(key)

        if self.checkPlotStatus():
            # print("Key selected: %s for %s" % (key, self.__plot_cases))
            data = EnsembleSummaryPlot().getPlotDataForKeyAndCases(self.__data_type_key, self.__plot_cases)
            self.plot_panel.setPlotData(data)
            self.plot_overview_panel.setPlotData(data)
            self.plot_debug_panel.setPlotData(data)