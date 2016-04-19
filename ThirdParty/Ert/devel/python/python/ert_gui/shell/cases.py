from ert.enkf import EnkfVarType

from ert_gui.shell import assertConfigLoaded, ErtShellCollection
from ert_gui.shell.libshell import autoCompleteList, splitArguments
from ert_gui.shell.libshell.shell_tools import boolValidator


class Cases(ErtShellCollection):
    def __init__(self, parent):
        super(Cases, self).__init__("case", parent)
        self._show_hidden = False

        self.addShellFunction(name="list",
                              function=Cases.list,
                              help_message="Shows a list of all available cases.")

        self.addShellFunction(name="select",
                              function=Cases.select,
                              completer=Cases.completeSelect,
                              help_arguments="<case_name>",
                              help_message="Change the current file system to the named case.")

        self.addShellFunction(name="create",
                              function=Cases.create,
                              help_arguments="<case_name>",
                              help_message="Create a new case with the specified named.")

        self.addShellFunction(name="summary_key_set",
                              function=Cases.summaryKeySet,
                              help_message="Shows a list of the stored summary keys.")

        self.addShellFunction(name="state",
                              function=Cases.state,
                              completer=Cases.completeFilesystem,
                              help_arguments="[case_name]",
                              help_message="Shows a list of the states of the individual realizations. "
                                           "Uses the current case if no case name is provided.")

        self.addShellFunction(name="time_map",
                              function=Cases.timemap,
                              completer=Cases.completeFilesystem,
                              help_arguments="[case_name]",
                              help_message="Shows a list of the time/report steps of the case. "
                                           "Uses the current case if no case name is provided.")

        self.addShellFunction(name="initialize",
                              function=Cases.initialize,
                              completer=Cases.completeFilesystem,
                              help_arguments="[case_name]",
                              help_message="Initialize the selected case from scratch. "
                                           "Uses the current if no case name is provided")

        self.addShellProperty(name="show_hidden",
                              getter=Cases.showHidden,
                              setter=Cases.setShowHidden,
                              validator=boolValidator,
                              completer=["true", "false"],
                              help_arguments="[true|false]",
                              help_message="Show or set the visibility of hidden cases",
                              pretty_attribute="Hidden case visibility")


    def showHidden(self):
        return self._show_hidden

    def setShowHidden(self, show_hidden):
        self._show_hidden = show_hidden

    @assertConfigLoaded
    def list(self, line):
        fs_list = self.getFileSystemNames()
        current_fs = self.ert().getEnkfFsManager().getCurrentFileSystem().getCaseName()
        max_length = max([len(fs) for fs in fs_list])
        case_format = "%1s %-" + str(max_length) + "s  %s"
        for fs in fs_list:
            current = ""
            if fs == current_fs:
                current = "*"

            state = "No Data"
            if self.ert().getEnkfFsManager().caseHasData(fs):
                state = "Data"

            print(case_format % (current, fs, state))

    def getFileSystemNames(self):
        fsm = self.ert().getEnkfFsManager()
        if self._show_hidden:
            return [case for case in fsm.getCaseList()]
        else:
            return [case for case in fsm.getCaseList() if not fsm.isCaseHidden(case)]

    @assertConfigLoaded
    def select(self, case_name):
        case_name = case_name.strip()
        if case_name in self.getFileSystemNames():
            fs = self.ert().getEnkfFsManager().getFileSystem(case_name)
            self.ert().getEnkfFsManager().switchFileSystem(fs)
        else:
            self.lastCommandFailed("Unknown case '%s'" % case_name)

    @assertConfigLoaded
    def completeSelect(self, text, line, begidx, endidx):
        return autoCompleteList(text, self.getFileSystemNames())

    @assertConfigLoaded
    def create(self, line):
        arguments = splitArguments(line)

        if len(arguments) == 1:
            case_name = arguments[0]
            if case_name not in self.getFileSystemNames():
                fs = self.ert().getEnkfFsManager().getFileSystem(case_name)
                self.ert().getEnkfFsManager().switchFileSystem(fs)
            else:
                self.lastCommandFailed("Case '%s' already exists!" % case_name)
        else:
            self.lastCommandFailed("Expected one argument: <case_name> received: '%s'" % line)

    @assertConfigLoaded
    def summaryKeySet(self, line):
        fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        key_set = sorted([key for key in fs.getSummaryKeySet().keys()])
        self.columnize(key_set)

    @assertConfigLoaded
    def state(self, case_name):
        case_name = case_name.strip()
        if not case_name:
            case_name = self.ert().getEnkfFsManager().getCurrentFileSystem().getCaseName()
        elif not case_name in self.getFileSystemNames():
            self.lastCommandFailed("Unknown case name '%s'" % case_name)
            return False

        state_map = self.ert().getEnkfFsManager().getStateMapForCase(case_name)
        states = ["%d: %s" % (index, state) for index, state in enumerate(state_map)]

        self.columnize(states)

    @assertConfigLoaded
    def completeFilesystem(self, text, line, begidx, endidx):
        return autoCompleteList(text, self.getFileSystemNames())

    @assertConfigLoaded
    def timemap(self, case_name):
        case_name = case_name.strip()
        if not case_name:
            case_name = self.ert().getEnkfFsManager().getCurrentFileSystem().getCaseName()
        elif not case_name in self.getFileSystemNames():
            self.lastCommandFailed("Unknown case name '%s'" % case_name)
            return False

        time_map = self.ert().getEnkfFsManager().getTimeMapForCase(case_name)
        report_steps = ["%d: %s" % (index, report_step_time) for index, report_step_time in enumerate(time_map)]

        self.columnize(report_steps)


    @assertConfigLoaded
    def initialize(self, case_name):
        case_name = case_name.strip()
        if not case_name:
            case_name = self.ert().getEnkfFsManager().getCurrentFileSystem().getCaseName()
        elif not case_name in self.getFileSystemNames():
            self.lastCommandFailed("Unknown case name '%s'" % case_name)
            return False

        ert = self.ert()
        fs = ert.getEnkfFsManager().getFileSystem(case_name)
        size = self.ert().getEnsembleSize()
        parameters = ert.ensembleConfig().getKeylistFromVarType(EnkfVarType.PARAMETER)
        ert.getEnkfFsManager().initializeCaseFromScratch(fs , parameters, 0, size - 1)
        
        print("Case: '%s' initialized")
