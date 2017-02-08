from ert.enkf.enkf_fs_manager import naturalSortKey
from ert.enkf import PlotSettings as ErtPlotSettings
from ert_gui.plottery.plot_config import PlotConfig
from ert_gui.shell import assertConfigLoaded, ErtShellCollection
from ert_gui.shell.libshell import autoCompleteList, boolValidator, pathCompleter, splitArguments
from ert_gui.shell.libshell.shell_tools import matchItems


def plotPathValidator(model, line):
    arguments = splitArguments(line)

    if len(arguments) == 1:
        return arguments[0]  # todo: check if exists and is file or directory for example
    else:
        raise ValueError("Can only set one path. If you require spaces in your path, "
                         "surround it with quotes: \"path with space\".")


class PlotSettings(ErtShellCollection):
    def __init__(self, parent):
        super(PlotSettings, self).__init__("plot_settings", parent)
        
        self.__cases = None
        if self.ert():
            ert_ps = self.ert().plotConfig( )
        else:
            ert_ps = ErtPlotSettings( )

        self.__plot_config = PlotConfig( ert_ps )
        self.shellContext()["plot_settings"] = self

        self.addShellFunction(name="current",
                              function=PlotSettings.current,
                              help_message="Shows the selected plot source case(s).")

        self.addShellFunction(name="reset_title",
                              function=PlotSettings.resetTitle,
                              help_message="Reset plot title back to default.")

        self.addShellFunction(name="select",
                              function=PlotSettings.select,
                              completer=PlotSettings.completeSelect,
                              help_arguments="[case_1..case_n]",
                              help_message="Select one or more cases as default plot sources. Empty resets to current case.")

        self.addShellProperty(name="path",
                              getter=PlotSettings.getPath,
                              setter=PlotSettings.setPath,
                              validator=plotPathValidator,
                              completer=pathCompleter,
                              help_arguments="[path]",
                              help_message="Show or set the plot output path",
                              pretty_attribute="Plot output path")

        self.addShellProperty(name="title",
                              getter=PlotConfig.title,
                              setter=PlotConfig.setTitle,
                              help_arguments="[new_title]",
                              help_message="Show or set the title of the plot",
                              pretty_attribute="Title",
                              model=self.__plot_config)

        self.addShellProperty(name="x_label",
                              getter=PlotConfig.xLabel,
                              setter=PlotConfig.setXLabel,
                              help_arguments="[new_label]",
                              help_message="Show or set the X label of the plot",
                              pretty_attribute="X label",
                              model=self.__plot_config)

        self.addShellProperty(name="y_label",
                              getter=PlotConfig.yLabel,
                              setter=PlotConfig.setYLabel,
                              help_arguments="[new_label]",
                              help_message="Show or set the Y label of the plot",
                              pretty_attribute="Y label",
                              model=self.__plot_config)

        self.addShellProperty(name="grid",
                              getter=PlotConfig.isGridEnabled,
                              setter=PlotConfig.setGridEnabled,
                              validator=boolValidator,
                              completer=["true", "false"],
                              help_arguments="[true|false]",
                              help_message="Show or set the grid visibility",
                              pretty_attribute="Grid visibility",
                              model=self.__plot_config)

        self.addShellProperty(name="legend",
                              getter=PlotConfig.isLegendEnabled,
                              setter=PlotConfig.setLegendEnabled,
                              validator=boolValidator,
                              completer=["true", "false"],
                              help_arguments="[true|false]",
                              help_message="Show or set the legend visibility",
                              pretty_attribute="Legend visibility",
                              model=self.__plot_config)

        self.addShellProperty(name="connection_lines",
                              getter=PlotConfig.isDistributionLineEnabled,
                              setter=PlotConfig.setDistributionLineEnabled,
                              validator=boolValidator,
                              completer=["true", "false"],
                              help_arguments="[true|false]",
                              help_message="Show or set the connection lines visibility",
                              pretty_attribute="Connection Line visibility",
                              model=self.__plot_config)

        self.addShellProperty(name="refcase",
                              getter=PlotConfig.isRefcaseEnabled,
                              setter=PlotConfig.setRefcaseEnabled,
                              validator=boolValidator,
                              completer=["true", "false"],
                              help_arguments="[true|false]",
                              help_message="Show or set the refcase visibility",
                              pretty_attribute="Refcase visibility",
                              model=self.__plot_config)

        self.addShellProperty(name="observations",
                              getter=PlotConfig.isObservationsEnabled,
                              setter=PlotConfig.setObservationsEnabled,
                              validator=boolValidator,
                              completer=["true", "false"],
                              help_arguments="[true|false]",
                              help_message="Show or set the observations visibility",
                              pretty_attribute="Observations visibility",
                              model=self.__plot_config)


    def getCurrentPlotCases(self):
        """ @rtype: list of str """

        if self.__cases is None:
            case_name = self.ert().getEnkfFsManager().getCurrentFileSystem().getCaseName()
            return [case_name]

        return self.__cases

    def plotConfig(self):
        return self.__plot_config

    @assertConfigLoaded
    def current(self, line):
        keys = self.getCurrentPlotCases()
        self.columnize(keys)

    @assertConfigLoaded
    def select(self, line):
        matched_cases = matchItems(line, self.getAllCaseList())

        if len(matched_cases) > 0:
            self.__cases = sorted(list(matched_cases), key=naturalSortKey)
        else:
            if len(line) > 0:
                self.lastCommandFailed("No valid case names provided: %s" % line)
            print("Case reset to default: %s" % self.ert().getEnkfFsManager().getCurrentFileSystem().getCaseName())
            self.__cases = None

    @assertConfigLoaded
    def completeSelect(self, text, line, begidx, endidx):
        return autoCompleteList(text, self.getAllCaseList())

    def getAllCaseList(self):
        fs_manager = self.ert().getEnkfFsManager()
        all_case_list = fs_manager.getCaseList()
        all_case_list = [case for case in all_case_list if not case.startswith(".")]
        return all_case_list

    @assertConfigLoaded
    def getPath(self):
        return self.ert().plotConfig().getPath()

    @assertConfigLoaded
    def setPath(self, path):
        self.ert().plotConfig().setPath(path)

    def resetTitle(self, line):
        self.plotConfig().setTitle(None)
