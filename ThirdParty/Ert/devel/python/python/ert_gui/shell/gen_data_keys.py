from ert.enkf import ErtImplType, EnkfObservationImplementationType
from ert.enkf.export import GenDataCollector
from ert.enkf.export.gen_data_observation_collector import GenDataObservationCollector
from ert_gui.shell import ShellFunction, extractFullArgument, autoCompleteListWithSeparator, ShellPlot, \
    assertConfigLoaded
from ert_gui.shell.shell_tools import matchItems


class GenDataKeys(ShellFunction):
    def __init__(self, shell_context):
        super(GenDataKeys, self).__init__("gen_data", shell_context)
        self.addHelpFunction("list", None, "Shows a list of all available gen_data keys.")
        self.addHelpFunction("plot", "<key_1> [key_2..key_n]", "Plot the specified key(s).")
        self.addHelpFunction("print", "<key_1> [key_2..key_n]", "Print the values for the specified key(s).")


    def fetchSupportedKeys(self):
        gen_data_keys = self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.GEN_DATA)
        gen_data_list = []
        for key in gen_data_keys:
            enkf_config_node = self.ert().ensembleConfig().getNode(key)
            gen_data_config = enkf_config_node.getDataModelConfig()

            for report_step in range(self.ert().getHistoryLength() + 1):
                if gen_data_config.hasReportStep(report_step):
                    gen_data_list.append("%s@%d" % (key, report_step))

        return gen_data_list


    @assertConfigLoaded
    def do_list(self, line):
        keys = sorted(self.fetchSupportedKeys())

        self.columnize(keys)


    @assertConfigLoaded
    def do_plot(self, line):
        keys = matchItems(line, self.fetchSupportedKeys())

        if len(keys) == 0:
            self.lastCommandFailed("Must have at least one GenData key")
            return False

        case_list = self.shellContext()["plot_settings"].getCurrentPlotCases()

        for key in keys:
            key, report_step = key.split("@", 1)
            report_step = int(report_step)
            plot = ShellPlot("%s at report step: %d" %(key, report_step))
            for case_name in case_list:
                data = GenDataCollector.loadGenData(self.ert(), case_name, key, report_step)

                if not data.empty:
                    plot.plotGenData(data, legend_label=case_name)

                obs_key = GenDataObservationCollector.getObservationKeyForDataKey(self.ert(), key, report_step)

                if obs_key is not None:
                    obs_data = GenDataObservationCollector.loadGenDataObservations(self.ert(), case_name, [obs_key])

                    if not obs_data.empty:
                        plot.plotObservations(obs_data, obs_key)

            plot.showLegend()


    @assertConfigLoaded
    def complete_plot(self, text, line, begidx, endidx):
        key = extractFullArgument(line, endidx)
        return autoCompleteListWithSeparator(key, self.fetchSupportedKeys())

    @assertConfigLoaded
    def do_print(self, line):
        keys = matchItems(line, self.fetchSupportedKeys())

        if len(keys) == 0:
            self.lastCommandFailed("Must have at least one GenData key")
            return False

        case_name = self.ert().getEnkfFsManager().getCurrentFileSystem().getCaseName()

        for key in keys:
            key, report_step = key.split("@", 1)
            report_step = int(report_step)
            data = GenDataCollector.loadGenData(self.ert(), case_name, key, report_step)
            print(data)


    @assertConfigLoaded
    def complete_print(self, text, line, begidx, endidx):
        key = extractFullArgument(line, endidx)
        return autoCompleteListWithSeparator(key, self.fetchSupportedKeys())