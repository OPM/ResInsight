import ert_gui.plottery.plots as plots

import matplotlib.pyplot as plt

from ert_gui.plottery.plot_config_factory import PlotConfigFactory
from ert_gui.shell import assertConfigLoaded
from ert_gui.plottery import PlotConfig, PlotContext
from ert_gui.shell.libshell import matchItems, extractFullArgument, autoCompleteListWithSeparator


class ShellPlot(object):
    @classmethod
    def _createPlotContext(cls, shell_context, data_gatherer, key):
        """
        :type shell_context: ShellContext
        :param data_gatherer: PlotDataGatherer
        :param key: str
        """
        figure = plt.figure()
        figure.set_tight_layout(True)
        cases = shell_context["plot_settings"].getCurrentPlotCases()

        plot_config = PlotConfig(key)
        #plot settings should keep of track of single attributes and only apply the changed ones...
        plot_config.copyConfigFrom(shell_context["plot_settings"].plotConfig())

        if plot_config.isUnnamed():
            plot_config.setTitle(key)

        #Apply data type specific changes for statistics...
        PlotConfigFactory.updatePlotConfigForKey(shell_context.ert(), key, plot_config)

        plot_context = PlotContext(shell_context.ert(), figure, plot_config, cases, key, data_gatherer)
        return plot_context

    @classmethod
    def plotEnsemble(cls, shell_context, data_gatherer, key):
        """
        :type shell_context: ShellContext
        :param data_gatherer: PlotDataGatherer
        :param key: str
        """
        plot_context = cls._createPlotContext(shell_context, data_gatherer, key)
        plots.plotEnsemble(plot_context)

    @classmethod
    def plotQuantiles(cls, shell_context, data_gatherer, key):
        """
        :type shell_context: ShellContext
        :param data_gatherer: PlotDataGatherer
        :param key: str
        """
        plot_context = cls._createPlotContext(shell_context, data_gatherer, key)
        plots.plotStatistics(plot_context)

    @classmethod
    def plotHistogram(cls, shell_context, data_gatherer, key):
        """
        :type shell_context: ShellContext
        :param data_gatherer: PlotDataGatherer
        :param key: str
        """
        plot_context = cls._createPlotContext(shell_context, data_gatherer, key)
        plots.plotHistogram(plot_context)

    @classmethod
    def plotDistribution(cls, shell_context, data_gatherer, key):
        """
        :type shell_context: ShellContext
        :param data_gatherer: PlotDataGatherer
        :param key: str
        """
        plot_context = cls._createPlotContext(shell_context, data_gatherer, key)
        plots.plotDistribution(plot_context)

    @classmethod
    def plotGaussianKDE(cls, shell_context, data_gatherer, key):
        """
        :type shell_context: ShellContext
        :param data_gatherer: PlotDataGatherer
        :param key: str
        """
        plot_context = cls._createPlotContext(shell_context, data_gatherer, key)
        plots.plotGaussianKDE(plot_context)

    @classmethod
    def plotCrossCaseStatistics(cls, shell_context, data_gatherer, key):
        """
        :type shell_context: ShellContext
        :param data_gatherer: PlotDataGatherer
        :param key: str
        """
        plot_context = cls._createPlotContext(shell_context, data_gatherer, key)
        plots.plotCrossCaseStatistics(plot_context)

    @classmethod
    def _checkForRequiredMethods(cls, instance):
        if not hasattr(instance, "fetchSupportedKeys"):
            raise NotImplementedError("Class must implement: fetchSupportedKeys()")

        if not hasattr(instance, "plotDataGatherer"):
            raise NotImplementedError("Class must implement: plotDataGatherer()")

    @classmethod
    def _createDoFunction(cls, plot_function, name):
        def do_function(self, line):
            keys = matchItems(line, self.fetchSupportedKeys())

            if len(keys) == 0:
                self.lastCommandFailed("Must have at least one %s key" % name)
                return False

            for key in keys:
                pdg = self.plotDataGatherer()
                plot_function(self.shellContext(), pdg, key)

        return assertConfigLoaded(do_function)

    @classmethod
    def _createCompleteFunction(cls):
        def complete_function(self, text, line, begidx, endidx):
            key = extractFullArgument(line, endidx)
            return autoCompleteListWithSeparator(key, self.fetchSupportedKeys())

        complete_function = assertConfigLoaded(complete_function)
        return complete_function

    @classmethod
    def addHistogramPlotSupport(cls, instance, name):
        """
        :type instance: ert_gui.shell.libshell.ShellCollection
        """
        cls._checkForRequiredMethods(instance)

        instance.addShellFunction(name="histogram",
                                  function=cls._createDoFunction(ShellPlot.plotHistogram, name),
                                  completer=cls._createCompleteFunction(),
                                  help_arguments="<key_1> [key_2..key_n]",
                                  help_message="Plot a histogram for the specified %s key(s)." % name)

    @classmethod
    def addGaussianKDEPlotSupport(cls, instance, name):
        """
        :type instance: ert_gui.shell.ShellFunction
        """
        cls._checkForRequiredMethods(instance)

        instance.addShellFunction(name="density",
                                  function=cls._createDoFunction(ShellPlot.plotGaussianKDE, name),
                                  completer=cls._createCompleteFunction(),
                                  help_arguments="<key_1> [key_2..key_n]",
                                  help_message="Plot a GaussianKDE plot for the specified %s key(s)." % name)

    @classmethod
    def addEnsemblePlotSupport(cls, instance, name):
        """
        :type instance: ert_gui.shell.ShellFunction
        """
        cls._checkForRequiredMethods(instance)

        instance.addShellFunction(name="plot",
                                  function=cls._createDoFunction(ShellPlot.plotEnsemble, name),
                                  completer=cls._createCompleteFunction(),
                                  help_arguments="<key_1> [key_2..key_n]",
                                  help_message="Plot an ensemble plot for the specified %s key(s)." % name)

    @classmethod
    def addQuantilesPlotSupport(cls, instance, name):
        """
        :type instance: ert_gui.shell.ShellFunction
        """
        cls._checkForRequiredMethods(instance)

        instance.addShellFunction(name="plot_quantile",
                                  function=cls._createDoFunction(ShellPlot.plotQuantiles, name),
                                  completer=cls._createCompleteFunction(),
                                  help_arguments="<key_1> [key_2..key_n]",
                                  help_message="Plot a different statistics for the specified %s key(s)." % name)

    @classmethod
    def addDistributionPlotSupport(cls, instance, name):
        """
        :type instance: ert_gui.shell.ShellFunction
        """
        cls._checkForRequiredMethods(instance)
        instance.addShellFunction(name="distribution",
                                  function=cls._createDoFunction(ShellPlot.plotDistribution, name),
                                  completer=cls._createCompleteFunction(),
                                  help_arguments="<key_1> [key_2..key_n]",
                                  help_message="Plot the distribution plot for the specified %s key(s)." % name)

    @classmethod
    def addCrossCaseStatisticsPlotSupport(cls, instance, name):
        """
        :type instance: ert_gui.shell.ShellFunction
        """
        cls._checkForRequiredMethods(instance)
        instance.addShellFunction(name="cross_case_statistics",
                                  function=cls._createDoFunction(ShellPlot.plotCrossCaseStatistics, name),
                                  completer=cls._createCompleteFunction(),
                                  help_arguments="<key_1> [key_2..key_n]",
                                  help_message="Plot the cross case statistics plot for the specified %s key(s)." % name)

    @classmethod
    def _createDoPrintFunction(cls, name):
        def do_function(self, line):
            keys = matchItems(line, self.fetchSupportedKeys())

            if len(keys) == 0:
                self.lastCommandFailed("Must have at least one %s key" % name)
                return False

            case_name = self.shellContext().ert().getEnkfFsManager().getCurrentFileSystem().getCaseName()

            for key in keys:
                pdg = self.plotDataGatherer()
                if pdg.canGatherDataForKey(key):
                    data = pdg.gatherData(self.shellContext().ert(), case_name, key)
                    print(data)
                else:
                    self.lastCommandFailed("Unable to print data for key: %s" % key)

        return assertConfigLoaded(do_function)

    @classmethod
    def addPrintSupport(cls, instance, name):
        """
        :type instance: ert_gui.shell.ShellFunction
        """
        cls._checkForRequiredMethods(instance)

        instance.addShellFunction(name="print",
                                  function=cls._createDoPrintFunction(name),
                                  completer=cls._createCompleteFunction(),
                                  help_arguments="<key_1> [key_2..key_n]",
                                  help_message="Print the values for the specified %s key(s)." % name)
