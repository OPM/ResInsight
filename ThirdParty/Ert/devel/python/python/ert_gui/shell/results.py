from ert.util import BoolVector
from ert_gui.shell import assertConfigLoaded, ErtShellCollection
from ert_gui.shell.libshell import splitArguments


class Results(ErtShellCollection):
    def __init__(self, parent):
        super(Results, self).__init__("results", parent)

        self.addShellFunction(name="runpath", function=Results.runpath, help_message="Shows the current runpath.")
        self.addShellFunction(name="load", function=Results.load, completer=Results.completeLoad, help_arguments="<realizations>", help_message="Load results from the specified realizations.") #todo iterations

    @assertConfigLoaded
    def runpath(self, args):
        runpath = self.ert().getModelConfig().getRunpathAsString()
        print("Runpath set to: %s" % runpath)


    @assertConfigLoaded
    def load(self, args):
        arguments = splitArguments(args)

        if len(arguments) < 1:
            self.lastCommandFailed("Loading requires a realization mask.")
            return False

        realization_count = self.ert().getEnsembleSize()

        mask = BoolVector(False, realization_count)
        mask_success = mask.updateActiveMask(arguments[0])

        if not mask_success:
            self.lastCommandFailed("The realization mask: '%s' is not valid." % arguments[0])
            return False

        fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        self.ert().loadFromForwardModel(mask, 0, fs)


    @assertConfigLoaded
    def completeLoad(self, text, line, begidx, endidx):
        arguments = splitArguments(line)

        if len(arguments) > 2 or len(arguments) == 2 and not text:
            return []

        if not text:
            return ["0-%d" % self.ert().getEnsembleSize()] # todo should generate based on realization directories.

        return []
