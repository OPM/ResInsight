from ert_gui.shell.libshell import ShellContext


class ErtShellContext(ShellContext):
    def __init__(self, shell):
        super(ErtShellContext, self).__init__(shell)
        self.__ert = None
        """ :type: EnKFMain """

    def ert(self):
        """ @rtype: ert.enkf.enkf_main.EnKFMain """
        return self.__ert

    def setErt(self, ert):
        """ @type ert: ert.enkf.enkf_main.EnKFMain """
        if self.__ert is not None and self.__ert != ert:
            self.__ert.free()
            self.__ert = None

        self.__ert = ert