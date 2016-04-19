from ert_gui.shell import assertConfigLoaded, ShellPlot, ErtShellCollection
from ert_gui.plottery import PlotDataGatherer as PDG


class CustomKWKeys(ErtShellCollection):
    def __init__(self, parent):
        super(CustomKWKeys, self).__init__("custom_kw", parent)

        self.addShellFunction(**{"name": "list",
                                 "function": CustomKWKeys.list,
                                 "help_message": "List all CustomKW keys."})

        self.__plot_data_gatherer = None

        ShellPlot.addPrintSupport(self, "CustomKW")
        ShellPlot.addHistogramPlotSupport(self, "CustomKW")
        ShellPlot.addGaussianKDEPlotSupport(self, "CustomKW")
        ShellPlot.addDistributionPlotSupport(self, "CustomKW")
        ShellPlot.addCrossCaseStatisticsPlotSupport(self, "CustomKW")


    def fetchSupportedKeys(self):
        return self.ert().getKeyManager().customKwKeys()


    def plotDataGatherer(self):
        if self.__plot_data_gatherer is None:
            custom_kw_pdg = PDG.gatherCustomKwData
            custom_kw_key_manager = self.ert().getKeyManager().isCustomKwKey
            self.__plot_data_gatherer = PDG(custom_kw_pdg, custom_kw_key_manager)

        return self.__plot_data_gatherer


    @assertConfigLoaded
    def list(self, line):
        self.columnize(self.fetchSupportedKeys())
