from ert_gui.plottery import PlotDataGatherer as PDG
from ert_gui.shell import ShellPlot, assertConfigLoaded, ErtShellCollection


class GenDataKeys(ErtShellCollection):
    def __init__(self, parent):
        super(GenDataKeys, self).__init__("gen_data", parent)
        self.addShellFunction(name="list", function=GenDataKeys.list, help_message="Shows a list of all available GenData keys.")

        self.__plot_data_gatherer = None

        ShellPlot.addPrintSupport(self, "GenData")
        ShellPlot.addEnsemblePlotSupport(self, "GenData")
        ShellPlot.addQuantilesPlotSupport(self, "GenData")


    def fetchSupportedKeys(self):
        return self.ert().getKeyManager().genDataKeys()

    def plotDataGatherer(self):
        if self.__plot_data_gatherer is None:
            gen_data_pdg = PDG.gatherGenDataData
            gen_data_key_manager = self.ert().getKeyManager().isGenDataKey
            gen_data_observation_pdg = PDG.gatherGenDataObservationData
            pdg = PDG(gen_data_pdg, gen_data_key_manager, observationGatherFunc=gen_data_observation_pdg)
            self.__plot_data_gatherer = pdg

        return self.__plot_data_gatherer

    @assertConfigLoaded
    def list(self, line):
        self.columnize(self.fetchSupportedKeys())
