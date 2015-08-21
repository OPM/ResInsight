from ert_gui.shell import ShellFunction, autoCompleteList, assertConfigLoaded


class Cases(ShellFunction):
    def __init__(self, shell_context):
        super(Cases, self).__init__("case", shell_context)

        self.addHelpFunction("list", None, "Shows a list of all available cases.")
        self.addHelpFunction("select", "<case_name>", "Change the current file system to the named case.")
        self.addHelpFunction("create", "<case_name>", "Create a new case with the specified named.")
        self.addHelpFunction("summary_key_set", None, "Shows a list of the stored summary keys.")
        self.addHelpFunction("state", "[case_name]", "Shows a list of the states of the individual realizations. "
                                                     "Uses the current case if no case name is provided.")
        self.addHelpFunction("time_map", "[case_name]", "Shows a list of the time/report steps of the case. "
                                                        "Uses the current case if no case name is provided.")

    @assertConfigLoaded
    def do_list(self, line):
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
        return sorted([fs for fs in self.ert().getEnkfFsManager().getCaseList()])


    @assertConfigLoaded
    def do_select(self, case_name):
        case_name = case_name.strip()
        if case_name in self.getFileSystemNames():
            fs = self.ert().getEnkfFsManager().getFileSystem(case_name)
            self.ert().getEnkfFsManager().switchFileSystem(fs)
        else:
            self.lastCommandFailed("Unknown case '%s'" % case_name)

    @assertConfigLoaded
    def complete_select(self, text, line, begidx, endidx):
        return autoCompleteList(text, self.getFileSystemNames())


    @assertConfigLoaded
    def do_create(self, line):
        arguments = self.splitArguments(line)

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
    def do_summary_key_set(self, line):
        fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        key_set = sorted([key for key in fs.getSummaryKeySet().keys()])
        self.columnize(key_set)

    @assertConfigLoaded
    def do_state(self, case_name):
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
    def complete_state(self, text, line, begidx, endidx):
        return autoCompleteList(text, self.getFileSystemNames())

    @assertConfigLoaded
    def do_time_map(self, case_name):
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
    def complete_time_map(self, text, line, begidx, endidx):
        return autoCompleteList(text, self.getFileSystemNames())

