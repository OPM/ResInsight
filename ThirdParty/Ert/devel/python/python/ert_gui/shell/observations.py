import os
from ert_gui.shell import ShellFunction, assertConfigLoaded, extractFullArgument, getPossibleFilenameCompletions


class Observations(ShellFunction):
    def __init__(self, shell_context):
        super(Observations, self).__init__("observations", shell_context)
        self.addHelpFunction("list", None, "List all observation keys.")
        self.addHelpFunction("clear", None, "Remove all observations.")
        self.addHelpFunction("load", "<observations_file>", "Add observations from the specified file.")

    @assertConfigLoaded
    def do_list(self, line):
        keys = [obs_vector.getObservationKey() for obs_vector in self.shellContext().ert().getObservations()]
        print("Observation keys:")
        self.columnize(keys)


    @assertConfigLoaded
    def do_clear(self, line):
        self.shellContext().ert().getObservations().clear()

    @assertConfigLoaded
    def do_load(self, config_file):
        if os.path.exists(config_file) and os.path.isfile(config_file):
            self.shellContext().ert().getObservations().load(config_file)
        else:
            self.lastCommandFailed("Observations file '%s' not found!\n" % config_file)

    @assertConfigLoaded
    def complete_load(self, text, line, begidx, endidx):
        argument = extractFullArgument(line, endidx)
        return getPossibleFilenameCompletions(argument)
