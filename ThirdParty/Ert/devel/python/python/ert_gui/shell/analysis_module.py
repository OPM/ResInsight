from ert_gui.shell import assertConfigLoaded, ErtShellCollection
from ert_gui.shell.libshell import extractFullArgument, autoCompleteListWithSeparator, matchItems, splitArguments


class AnalysisModule(ErtShellCollection):
    def __init__(self, parent):
        super(AnalysisModule, self).__init__("analysis_module", parent)

        self.addShellProperty(name="active_module",
                              getter=AnalysisModule.getAnalysisModule,
                              setter=AnalysisModule.setAnalysisModule,
                              validator=AnalysisModule.validateAnalysisModule,
                              completer=AnalysisModule.completeAnalysisModule,
                              help_arguments="[analysis_module]",
                              help_message="Show or set the current analysis module.",
                              pretty_attribute="Active Module")

        self.addShellFunction(name="list",
                              function=AnalysisModule.list,
                              help_message="Shows a list of the available analysis modules.")

        self.addShellFunction(name="variables",
                              function=AnalysisModule.variables,
                              help_message="Shows a list of the available options for the current analysis module.")

        self.addShellFunction(name="set",
                              function=AnalysisModule.set,
                              completer=AnalysisModule.completeSet,
                              help_arguments="<variable_name> <value>",
                              help_message="Set a variable value.")


    @assertConfigLoaded
    def getAnalysisModule(self):
        return self.ert().analysisConfig().activeModuleName()


    @assertConfigLoaded
    def setAnalysisModule(self, analysis_module_name):
        self.ert().analysisConfig().selectModule(analysis_module_name)


    @assertConfigLoaded
    def validateAnalysisModule(self, line):
        keys = matchItems(line, self.getAnalysisModules())

        if len(keys) == 0 or len(keys) > 1:
            raise ValueError("Must enter a single valid Analysis Module")

        return list(keys)[0]


    def completeAnalysisModule(self, text, line, begidx, endidx):
        key = extractFullArgument(line, endidx)
        return autoCompleteListWithSeparator(key, self.getAnalysisModules())


    @assertConfigLoaded
    def getAnalysisModules(self):
        analysis_modules = self.ert().analysisConfig().getModuleList()
        items = [analysis_module for analysis_module in analysis_modules]
        return items


    @assertConfigLoaded
    def list(self, args):
        items = self.getAnalysisModules()
        self.columnize(items)


    @assertConfigLoaded
    def variables(self, args):
        active_module = self.ert().analysisConfig().getActiveModule()
        variables = active_module.getVariableNames()

        format = " %-20s %-6s %-20s %s"
        print(format % ("Name", "Type", "Value", "Description"))

        for variable_name in variables:
            variable_type = active_module.getVariableType(variable_name).__name__
            variable_value = active_module.getVariableValue(variable_name)
            variable_description = active_module.getVariableDescription(variable_name)
            print(format % (variable_name, variable_type, variable_value, variable_description))


    @assertConfigLoaded
    def set(self, line):
        arguments = splitArguments(line)

        if len(arguments) > 1:
            active_module = self.ert().analysisConfig().getActiveModule()
            variables = active_module.getVariableNames()
            variable, argument = line.split(" ", 1)

            if not variable in variables:
                self.lastCommandFailed("Variable with name: %s, not available in analysis module!" % variable)
            else:
                variable_type = active_module.getVariableType(variable)
                try:
                    value = variable_type(argument)
                    active_module.setVar(variable, argument)
                except ValueError:
                    self.lastCommandFailed("Unable to convert '%s' into to a: %s" % (argument, variable_type.__name__))

        else:
            self.lastCommandFailed("This keyword requires a variable name and a value.")



    @assertConfigLoaded
    def completeSet(self, text, line, begidx, endidx):
        arguments = splitArguments(line)

        if len(arguments) > 2 or len(arguments) == 2 and not text:
            return []

        active_module = self.ert().analysisConfig().getActiveModule()
        variables = active_module.getVariableNames()
        return autoCompleteListWithSeparator(text, variables)