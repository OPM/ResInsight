import atexit
from cmd import Cmd
import readline
import os

from ert.enkf import EnKFMain
from ert_gui.shell import PlotSettings

from ert_gui.shell.analysis_module import AnalysisModule
from ert_gui.shell.custom_kw_keys import CustomKWKeys
from ert_gui.shell.debug import Debug
from ert_gui.shell.cases import Cases
from ert_gui.shell.export import Export
from ert_gui.shell.gen_data_keys import GenDataKeys
from ert_gui.shell.gen_kw_keys import GenKWKeys
from ert_gui.shell.results import Results
from ert_gui.shell.plugins import Plugins
from ert_gui.shell.simulations import Simulations
from ert_gui.shell.smoother import Smoother
from ert_gui.shell.storage import Storage
from ert_gui.shell.summary_keys import SummaryKeys
from ert_gui.shell.workflows import Workflows
from ert_gui.shell.observations import Observations
from ert_gui.shell.server import Server
from ert_gui.shell.libshell import extractFullArgument, getPossibleFilenameCompletions
from ert_gui.shell import ErtShellContext

import matplotlib

class ErtShell(Cmd):
    prompt = "--> "
    intro = " :::::::::::::::::::::::::::::::::::::\n" \
            " ::                                 ::\n" \
            " ::    ______   ______   _______    ::\n" \
            " ::   |  ____| |  __  \ |__   __|   ::\n" \
            " ::   | |__    | |__) |    | |      ::\n" \
            " ::   |  __|   |  _  /     | |      ::\n" \
            " ::   | |____  | | \ \     | |      ::\n" \
            " ::   |______| |_|  \_\    |_|      ::\n" \
            " ::                                 ::\n" \
            " ::  Ensemble based Reservoir Tool  ::\n" \
            " :::::::::::::::::::::::::::::::::::::\n" \
            "\n" \
            "Interactive shell for working with ERT.\n" \
            "\n" \
            "-- Type help for a list of supported commands.\n" \
            "-- Type exit or press Ctrl+D to end the shell session.\n" \
            "-- Press Tab for auto completion.\n" \
            "-- Arrow up/down for history.\n"


    def __init__(self, forget_history=False):
        Cmd.__init__(self)

        self.__children = []

        shell_context = ErtShellContext(self)
        self.__shell_context = shell_context

        if not forget_history:
            self.__history_file = os.path.join(os.path.expanduser("~/.ertshell/ertshell.history"))
            self.__init_history()
        else:
            self.__history_file = None

        matplotlib.rcParams["interactive"] = True
        matplotlib.rcParams["mathtext.default"] = "regular"
        matplotlib.rcParams["verbose.level"] = "helpful"
        matplotlib.rcParams["verbose.fileo"] = "sys.stderr"

        try:
            matplotlib.style.use("ggplot") # available from version 1.4
        except AttributeError:
            pass

        Debug(self)
        PlotSettings(self)
        Cases(self)
        Workflows(self)
        Plugins(self)
        SummaryKeys(self)
        GenDataKeys(self)
        GenKWKeys(self)
        Results(self)
        Simulations(self)
        CustomKWKeys(self)
        AnalysisModule(self)
        Smoother(self)
        Observations(self)
        Export(self)
        Storage(self)
        Server(self)

        self.__last_command_failed = False

        atexit.register(self._cleanup)

    def __init_history(self):
        try:
            readline.set_history_length(100)
            readline.read_history_file(self.__history_file)
        except IOError:
            pass
        atexit.register(self.__save_history)

    def __save_history(self):
        if self.__history_file is not None:
            if not os.path.exists(os.path.dirname(self.__history_file)):
                os.makedirs(os.path.dirname(self.__history_file))

            readline.write_history_file(self.__history_file)

    def _cleanup(self):
        print("Performing cleanup...")

        for child in self.__children:
            child.cleanup()

        if self.shellContext().ert() is not None:
            self.shellContext().setErt(None)


    def addChild(self, child):
        self.__children.append(child)

    def emptyline(self):
        pass

    def do_load_config(self, config_file):
        if os.path.exists(config_file) and os.path.isfile(config_file):
            self.shellContext().setErt(EnKFMain(config_file))
        else:
            self.lastCommandFailed("Config file '%s' not found!\n" % config_file)

    def complete_load_config(self, text, line, begidx, endidx):
        argument = extractFullArgument(line, endidx)
        return getPossibleFilenameCompletions(argument)


    def help_load_config(self):
        print("\n".join(("load_config config_file",
                         "    Loads a config file.")))

    def do_cwd(self, line):
        cwd = os.getcwd()
        print("Current directory: %s" % cwd)

    def help_cwd(self):
        print("Show the current directory.")

    def do_exit(self, line):
        return True

    def help_exit(self):
        return "\n".join(("exit",
                          "    End the shell session.")),

    do_EOF = do_exit

    def help_EOF(self):
        return "\n".join(("EOF",
                          "    The same as exit. (Ctrl+D)")),

    def shellContext(self):
        return self.__shell_context

    def default(self, line):
        Cmd.default(self, line)
        self.__last_command_failed = True

    def precmd(self, line):
        self.__last_command_failed = False
        return Cmd.precmd(self, line)

    def invokeCommand(self, line):
        self.__last_command_failed = False
        self.onecmd(line)
        return not self.__last_command_failed

    def lastCommandFailed(self, message):
        print("Error: %s" % message)
        self.__last_command_failed = True

    def get_names(self):
        return dir(self)






