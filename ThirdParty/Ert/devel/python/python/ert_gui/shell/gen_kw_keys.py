from ert.enkf.export.gen_kw_collector import GenKwCollector
from ert_gui.shell import ShellFunction, extractFullArgument, autoCompleteListWithSeparator, ShellPlot, \
    assertConfigLoaded
from ert_gui.shell.shell_tools import matchItems


class GenKWKeys(ShellFunction):
    def __init__(self, shell_context):
        super(GenKWKeys, self).__init__("gen_kw", shell_context)
        self.addHelpFunction("list", None, "Shows a list of all available gen_kw keys.")
        self.addHelpFunction("histogram", "<key_1> [key_2..key_n]", "Plot the histogram for the specified key(s).")
        self.addHelpFunction("density", "<key_1> [key_2..key_n]", "Plot the density for the specified key(s).")
        self.addHelpFunction("print", "<key_1> [key_2..key_n]", "Print the values for the specified key(s).")


    def fetchSupportedKeys(self):
        return GenKwCollector.getAllGenKwKeys(self.ert())

    @assertConfigLoaded
    def do_list(self, line):
        keys = sorted(self.fetchSupportedKeys())

        self.columnize(keys)

    @assertConfigLoaded
    def do_histogram(self, line):
        keys = matchItems(line, self.fetchSupportedKeys())

        if len(keys) == 0:
            self.lastCommandFailed("Must have at least one GenKW key")
            return False

        case_list = self.shellContext()["plot_settings"].getCurrentPlotCases()

        for key in keys:
            for case_name in case_list:
                data = GenKwCollector.loadAllGenKwData(self.ert(), case_name, [key])
                if not data.empty:
                    plot = ShellPlot(key)
                    plot.histogram(data, key, log_on_x=key.startswith("LOG10_"))

    @assertConfigLoaded
    def complete_histogram(self, text, line, begidx, endidx):
        key = extractFullArgument(line, endidx)
        return autoCompleteListWithSeparator(key, self.fetchSupportedKeys())


    @assertConfigLoaded
    def do_density(self, line):
        keys = matchItems(line, self.fetchSupportedKeys())

        if len(keys) == 0:
            self.lastCommandFailed("Must have at least one GenKW key")
            return False

        case_list = self.shellContext()["plot_settings"].getCurrentPlotCases()

        for key in keys:
            plot = ShellPlot(key)
            for case_name in case_list:
                data = GenKwCollector.loadAllGenKwData(self.ert(), case_name, [key])

                if not data.empty:
                    plot.density(data, key, legend_label=case_name)
            plot.showLegend()

    @assertConfigLoaded
    def complete_density(self, text, line, begidx, endidx):
        key = extractFullArgument(line, endidx)
        return autoCompleteListWithSeparator(key, self.fetchSupportedKeys())

    @assertConfigLoaded
    def do_print(self, line):
        keys = matchItems(line, self.fetchSupportedKeys())

        if len(keys) == 0:
            self.lastCommandFailed("Must have at least one GenKW key")
            return False

        case_name = self.ert().getEnkfFsManager().getCurrentFileSystem().getCaseName()

        data = GenKwCollector.loadAllGenKwData(self.ert(), case_name, keys)
        print(data)


    @assertConfigLoaded
    def complete_print(self, text, line, begidx, endidx):
        key = extractFullArgument(line, endidx)
        return autoCompleteListWithSeparator(key, self.fetchSupportedKeys())