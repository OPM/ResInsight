from ert_gui.shell import ShellPlot, assertConfigLoaded, ErtShellCollection
from ert_gui.plottery import PlotDataGatherer as PDG


class GenKWKeys(ErtShellCollection):
    def __init__(self, parent):
        super(GenKWKeys, self).__init__("gen_kw", parent)
        self.addShellFunction(name="list", function=GenKWKeys.list, help_message="Shows a list of all available GenKW keys.")

        self.__plot_data_gatherer = None

        ShellPlot.addPrintSupport(self, "GenKW")
        ShellPlot.addHistogramPlotSupport(self, "GenKW")
        ShellPlot.addGaussianKDEPlotSupport(self, "GenKW")
        ShellPlot.addDistributionPlotSupport(self, "GenKW")
        ShellPlot.addCrossCaseStatisticsPlotSupport(self, "GenKW")

    def fetchSupportedKeys(self):
        return self.ert().getKeyManager().genKwKeys()

    def plotDataGatherer(self):
        if self.__plot_data_gatherer is None:
            gen_kw_pdg = PDG.gatherGenKwData
            gen_kw_key_manager = self.ert().getKeyManager().isGenKwKey
            self.__plot_data_gatherer = PDG(gen_kw_pdg, gen_kw_key_manager)

        return self.__plot_data_gatherer

    @assertConfigLoaded
    def list(self, line):
        self.columnize(self.fetchSupportedKeys())
