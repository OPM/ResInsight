from PyQt4.QtCore import Qt
from PyQt4.QtGui import QMainWindow, QDockWidget, QTabWidget, QWidget, QVBoxLayout

from ert_gui.models.connectors.init import CaseSelectorModel
from ert_gui.plottery import PlotContext, PlotDataGatherer, PlotConfig, plots


from ert_gui.tools.plot import DataTypeKeysWidget, CaseSelectionWidget, CustomizePlotWidget, PlotWidget
from ert_gui.tools.plot import DataTypeKeysListModel
from ert_gui.widgets.util import may_take_a_long_time


class PlotWindow(QMainWindow):
    def __init__(self, ert, parent):
        QMainWindow.__init__(self, parent)

        self.__ert = ert
        """:type: ert.enkf.enkf_main.EnKFMain"""

        self.setMinimumWidth(750)
        self.setMinimumHeight(500)

        self.setWindowTitle("Plotting")
        self.activateWindow()

        self.__central_tab = QTabWidget()
        self.__central_tab.currentChanged.connect(self.currentPlotChanged)

        central_widget = QWidget()
        central_layout = QVBoxLayout()
        central_layout.setContentsMargins(0, 0, 0, 0)
        central_widget.setLayout(central_layout)

        central_layout.addWidget(self.__central_tab)

        self.setCentralWidget(central_widget)

        key_manager = ert.getKeyManager()
        """:type: ert.enkf.key_manager.KeyManager """

        self.__plot_widgets = []
        """:type: list of PlotWidget"""

        self.__data_gatherers = []
        """:type: list of PlotDataGatherer """

        PDG = PlotDataGatherer
        summary_gatherer = self.createDataGatherer(PDG.gatherSummaryData, key_manager.isSummaryKey, refcaseGatherFunc=PDG.gatherSummaryRefcaseData, observationGatherFunc=PDG.gatherSummaryObservationData)
        gen_data_gatherer = self.createDataGatherer(PDG.gatherGenDataData, key_manager.isGenDataKey, observationGatherFunc=PDG.gatherGenDataObservationData)
        gen_kw_gatherer = self.createDataGatherer(PDG.gatherGenKwData, key_manager.isGenKwKey)
        custom_kw_gatherer = self.createDataGatherer(PDG.gatherCustomKwData, key_manager.isCustomKwKey)


        self.addPlotWidget("Ensemble", plots.plotEnsemble, [summary_gatherer, gen_data_gatherer])
        self.addPlotWidget("Overview", plots.plotOverview, [summary_gatherer, gen_data_gatherer])
        self.addPlotWidget("Statistics", plots.plotStatistics, [summary_gatherer, gen_data_gatherer])
        self.addPlotWidget("Histogram", plots.plotHistogram, [gen_kw_gatherer, custom_kw_gatherer])
        self.addPlotWidget("Gaussian KDE", plots.plotGaussianKDE, [gen_kw_gatherer, custom_kw_gatherer])


        self.__data_types_key_model = DataTypeKeysListModel(ert)

        self.__data_type_keys_widget = DataTypeKeysWidget(self.__data_types_key_model)
        self.__data_type_keys_widget.dataTypeKeySelected.connect(self.keySelected)
        self.addDock("Data types", self.__data_type_keys_widget)

        current_case = CaseSelectorModel().getCurrentChoice()
        self.__case_selection_widget = CaseSelectionWidget(current_case)
        self.__case_selection_widget.caseSelectionChanged.connect(self.keySelected)
        plot_case_dock = self.addDock("Plot case", self.__case_selection_widget)

        self.__customize_plot_widget = CustomizePlotWidget()
        self.__customize_plot_widget.customPlotSettingsChanged.connect(self.keySelected)
        customize_plot_dock = self.addDock("Customize", self.__customize_plot_widget)


        self.tabifyDockWidget(plot_case_dock, customize_plot_dock)

        plot_case_dock.show()
        plot_case_dock.raise_()

        self.__plot_widgets[self.__central_tab.currentIndex()].setActive()
        self.__data_type_keys_widget.selectDefault()


    def createDataGatherer(self, dataGatherFunc, gatherConditionFunc, refcaseGatherFunc=None, observationGatherFunc=None):
        data_gatherer = PlotDataGatherer(dataGatherFunc, gatherConditionFunc, refcaseGatherFunc=refcaseGatherFunc, observationGatherFunc=observationGatherFunc)
        self.__data_gatherers.append(data_gatherer)
        return data_gatherer


    def currentPlotChanged(self):
        for plot_widget in self.__plot_widgets:
            plot_widget.setActive(False)
            index = self.__central_tab.indexOf(plot_widget)

            if index == self.__central_tab.currentIndex() and plot_widget.canPlotKey(self.getSelectedKey()):
                plot_widget.setActive()
                plot_widget.updatePlot()


    def createPlotContext(self, figure):
        key = self.getSelectedKey()
        cases = self.__case_selection_widget.getPlotCaseNames()
        data_gatherer = next((data_gatherer for data_gatherer in self.__data_gatherers if data_gatherer.canGatherDataForKey(key)), None)
        plot_config = PlotConfig(key)
        self.applyCustomization(plot_config)
        return PlotContext(self.__ert, figure, plot_config, cases, key, data_gatherer)

    def getSelectedKey(self):
        key = str(self.__data_type_keys_widget.getSelectedItem())
        return key

    def addPlotWidget(self, name, plotFunction, data_gatherers, enabled=True):
        plot_condition_function_list = [data_gatherer.canGatherDataForKey for data_gatherer in data_gatherers]
        plot_widget = PlotWidget(name, plotFunction, plot_condition_function_list, self.createPlotContext)

        index = self.__central_tab.addTab(plot_widget, name)
        self.__plot_widgets.append(plot_widget)
        self.__central_tab.setTabEnabled(index, enabled)


    def addDock(self, name, widget, area=Qt.LeftDockWidgetArea, allowed_areas=Qt.AllDockWidgetAreas):
        dock_widget = QDockWidget(name)
        dock_widget.setObjectName("%sDock" % name)
        dock_widget.setWidget(widget)
        dock_widget.setAllowedAreas(allowed_areas)
        dock_widget.setFeatures(QDockWidget.DockWidgetFloatable | QDockWidget.DockWidgetMovable)

        self.addDockWidget(area, dock_widget)
        return dock_widget


    def applyCustomization(self, plot_config):
        custom = self.__customize_plot_widget.getCustomSettings()

        plot_config.setObservationsEnabled(custom["show_observations"])
        plot_config.setRefcaseEnabled(custom["show_refcase"])
        plot_config.setLegendEnabled(custom["show_legend"])
        plot_config.setGridEnabled(custom["show_grid"])


    @may_take_a_long_time
    def keySelected(self):
        key = self.getSelectedKey()

        for plot_widget in self.__plot_widgets:
            plot_widget.setDirty()
            index = self.__central_tab.indexOf(plot_widget)
            self.__central_tab.setTabEnabled(index, plot_widget.canPlotKey(key))

        for plot_widget in self.__plot_widgets:
            if plot_widget.canPlotKey(key):
                plot_widget.updatePlot()
