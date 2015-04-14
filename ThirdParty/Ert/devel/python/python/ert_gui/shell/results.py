from ert.util import BoolVector
from ert_gui.shell import ShellFunction, assertConfigLoaded


class Results(ShellFunction):
    def __init__(self, shell_context):
        super(Results, self).__init__("results", shell_context)

        self.addHelpFunction("runpath", None, "Shows the current runpath.")
        self.addHelpFunction("load", "<realizations>", "Load results from the specified realizations.") #todo iterations

    @assertConfigLoaded
    def do_runpath(self, args):
        runpath = self.ert().getModelConfig().getRunpathAsString()
        print("Runpath set to: %s" % runpath)


    @assertConfigLoaded
    def do_load(self, args):
        arguments = self.splitArguments(args)

        if len(arguments) < 1:
            print("Error: Loading requires a realization mask.")
            return False

        realization_count = self.ert().getEnsembleSize()

        mask = BoolVector(False, realization_count)
        mask_success = BoolVector.updateActiveMask(arguments[0], mask)

        if not mask_success:
            print("Error: The realization mask: '%s' is not valid." % arguments[0])
            return False

        fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        self.ert().loadFromForwardModel(mask, 0, fs)


    @assertConfigLoaded
    def complete_load(self, text, line, begidx, endidx):
        arguments = self.splitArguments(line)

        if len(arguments) > 2 or len(arguments) == 2 and not text:
            return []

        if not text:
            return ["0-%d" % self.ert().getEnsembleSize()] # todo should generate based on realization directories.

        return []

