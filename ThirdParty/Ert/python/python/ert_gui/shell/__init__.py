from .ert_shell_context import ErtShellContext

def isConfigLoaded(shell_context, verbose=True):
    """ @rtype: bool """
    if shell_context.ert() is None:
        if verbose:
            print("Error: A config file has not been loaded!")
        return False
    return True

def assertConfigLoaded(func):
    def wrapper(self, *args, **kwargs):
        # prefixes should be either do_ or complete_
        if func.__name__.startswith("complete_"):
            result = []
            verbose = False
        else:
            result = False
            verbose = True

        if isConfigLoaded(self.shellContext(), verbose=verbose):
            result = func(self, *args, **kwargs)

        return result

    wrapper.__doc__ = func.__doc__
    wrapper.__name__ = func.__name__

    return wrapper

from .ert_shell_collection import ErtShellCollection
from .plot_settings import PlotSettings
from .shell_plot import ShellPlot
from .ertshell import ErtShell
