import os

from ert_gui.shell import assertConfigLoaded, ErtShellCollection
from ert_gui.shell.libshell import extractFullArgument, getPossibleFilenameCompletions


class Observations(ErtShellCollection):
    def __init__(self, parent):
        super(Observations, self).__init__("observations", parent)

        self.addShellFunction(name="list",
                              function=Observations.list,
                              help_message="List all observation keys.")

        self.addShellFunction(name="clear",
                              function=Observations.clear,
                              help_message="Remove all observations.")

        self.addShellFunction(name="load",
                              function=Observations.load,
                              completer=Observations.completeLoad,
                              help_arguments="<observations_file>",
                              help_message="Add observations from the specified file.")

        self.addShellFunction(name="reload",
                              function=Observations.reload,
                              completer=Observations.completeReload,
                              help_arguments="<observations_file>",
                              help_message="Perform a clear before adding observations from the specified file.")


    @assertConfigLoaded
    def list(self, line):
        keys = [obs_vector.getObservationKey() for obs_vector in self.ert().getObservations()]
        print("Observation keys:")
        self.columnize(keys)

    @assertConfigLoaded
    def clear(self, line):
        self.ert().getObservations().clear()

    @assertConfigLoaded
    def load(self, config_file):
        if os.path.exists(config_file) and os.path.isfile(config_file):
            # This will append observations; alternatively reload()
            # can be used to clear the current observations first.
            self.ert().loadObservations( config_file , clear = False )
        else:
            self.lastCommandFailed("Observations file '%s' not found!\n" % config_file)

    @assertConfigLoaded
    def reload(self, config_file):
        if os.path.exists(config_file) and os.path.isfile(config_file):
            # If clear is False the new observations will be added to the existing.
            self.ert().loadObservations( config_file , clear = True )
        else:
            self.lastCommandFailed("Observations file '%s' not found!\n" % config_file)

    @assertConfigLoaded
    def completeLoad(self, text, line, begidx, endidx):
        argument = extractFullArgument(line, endidx)
        return getPossibleFilenameCompletions(argument)

    @assertConfigLoaded
    def completeReload(self, text, line, begidx, endidx):
        argument = extractFullArgument(line, endidx)
        return getPossibleFilenameCompletions(argument)
