from ert.enkf import ErtImplType
from ert.enkf.export.summary_collector import SummaryCollector
from ert.enkf.export.summary_observation_collector import SummaryObservationCollector
from ert_gui.shell import ShellFunction, assertConfigLoaded, extractFullArgument, autoCompleteListWithSeparator, ShellPlot
from ert_gui.shell.shell_tools import matchItems


class SummaryKeys(ShellFunction):
    def __init__(self, shell_context):
        super(SummaryKeys, self).__init__("summary", shell_context)

        self.addHelpFunction("list", None, "Shows a list of all available summary keys. (* = with observations)")
        self.addHelpFunction("observations", None, "Shows a list of all available summary key observations.")
        self.addHelpFunction("matchers", None, "Shows a list of all summary keys that the ensemble will match "
                                               "against during simulations and manual load.")
        self.addHelpFunction("add_matcher", "<summary_key>", "Add a matcher to the summary key matcher set.")
        self.addHelpFunction("plot", "<key_1> [key_2..key_n]", "Plot the specified key(s).")
        self.addHelpFunction("plot_area", "<key_1> [key_2..key_n]", "Plot the area between the minimum and maximum for the specified key(s).")

    @assertConfigLoaded
    def do_list(self, line):
        ensemble_config = self.ert().ensembleConfig()
        keys = sorted([key for key in ensemble_config.getKeylistFromImplType(ErtImplType.SUMMARY)])
        observation_keys = [key for key in keys if len(ensemble_config.getNode(key).getObservationKeys()) > 0]

        result = ["*%s" % key if key in observation_keys else " %s" % key for key in keys]

        self.columnize(result)

    @assertConfigLoaded
    def do_observations(self, line):
        keys = self.summaryKeys()

        observation_keys = []
        for key in keys:
            obs_keys = self.ert().ensembleConfig().getNode(key).getObservationKeys()
            observation_keys.extend(obs_keys)

        self.columnize(observation_keys)

    @assertConfigLoaded
    def do_matchers(self, line):
        ensemble_config = self.ert().ensembleConfig()
        summary_key_matcher = ensemble_config.getSummaryKeyMatcher()
        keys = sorted(["*%s" % key if summary_key_matcher.isRequired(key) else " %s" % key for key in summary_key_matcher.keys()])

        self.columnize(keys)

    @assertConfigLoaded
    def do_add_matcher(self, line):
        args = self.splitArguments(line)

        if len(args) < 1:
            self.lastCommandFailed("A summary key is required.")
            return False


        self.ert().ensembleConfig().getSummaryKeyMatcher().addSummaryKey(args[0].strip())


    def summaryKeys(self):
        ensemble_config = self.ert().ensembleConfig()
        return sorted([key for key in ensemble_config.getKeylistFromImplType(ErtImplType.SUMMARY)])


    @assertConfigLoaded
    def do_plot(self, line):
        keys = matchItems(line, self.summaryKeys())

        if len(keys) == 0:
            self.lastCommandFailed("Must have at least one Summary key")
            return False

        case_list = self.shellContext()["plot_settings"].getCurrentPlotCases()

        for key in keys:
            plot = ShellPlot(key)
            for case_name in case_list:
                data = SummaryCollector.loadAllSummaryData(self.ert(), case_name, [key])
                if not data.empty:
                    plot.plot(data, value_column=key, legend_label=case_name)

            if len(case_list) > 0 and SummaryObservationCollector.summaryKeyHasObservations(self.ert(), key):
                observation_data = SummaryObservationCollector.loadObservationData(self.ert(), case_list[0], [key])

                if not observation_data.empty:
                    plot.plotObservations(observation_data, value_column=key)

            plot.showLegend()

    @assertConfigLoaded
    def complete_plot(self, text, line, begidx, endidx):
        key = extractFullArgument(line, endidx)
        return autoCompleteListWithSeparator(key, self.summaryKeys())

    @assertConfigLoaded
    def do_plot_area(self, line):
        keys = matchItems(line, self.summaryKeys())

        if len(keys) == 0:
            self.lastCommandFailed("Must have at least one Summary key")
            return False

        case_list = self.shellContext()["plot_settings"].getCurrentPlotCases()

        for key in keys:
            plot = ShellPlot(key)
            for case_name in case_list:
                data = SummaryCollector.loadAllSummaryData(self.ert(), case_name, [key])
                if not data.empty:
                    plot.plotArea(data, value_column=key, legend_label=case_name)

            if len(case_list) > 0 and SummaryObservationCollector.summaryKeyHasObservations(self.ert(), key):
                observation_data = SummaryObservationCollector.loadObservationData(self.ert(), case_list[0], [key])
                if not observation_data.empty:
                    plot.plotObservations(observation_data, value_column=key)

            plot.showLegend()

    @assertConfigLoaded
    def complete_plot_area(self, text, line, begidx, endidx):
        key = extractFullArgument(line, endidx)
        return autoCompleteListWithSeparator(key, self.summaryKeys())

    @assertConfigLoaded
    def do_plot_quantile(self, line):
        keys = matchItems(line, self.summaryKeys())

        if len(keys) == 0:
            self.lastCommandFailed("Must have at least one Summary key")
            return False

        case_list = self.shellContext()["plot_settings"].getCurrentPlotCases()

        for key in keys:
            plot = ShellPlot(key)
            for case_name in case_list:
                data = SummaryCollector.loadAllSummaryData(self.ert(), case_name, [key])
                if not data.empty:
                    plot.plotQuantiles(data, value_column=key, legend_label=case_name)

            if len(case_list) > 0 and SummaryObservationCollector.summaryKeyHasObservations(self.ert(), key):
                observation_data = SummaryObservationCollector.loadObservationData(self.ert(), case_list[0], [key])
                if not observation_data.empty:
                    plot.plotObservations(observation_data, value_column=key)

            plot.showLegend()

    @assertConfigLoaded
    def complete_plot_quantile(self, text, line, begidx, endidx):
        key = extractFullArgument(line, endidx)
        return autoCompleteListWithSeparator(key, self.summaryKeys())