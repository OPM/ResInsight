class ShellContext(object):
    def __init__(self, shell):
        super(ShellContext, self).__init__()

        self.__shell = shell
        """ :type: Cmd """

        self.__ert = None
        """ :type: EnKFMain """

        self.__settings = {}


    def ert(self):
        """ @rtype: ert.enkf.enkf_main.EnKFMain """
        return self.__ert

    def setErt(self, ert):
        """ @type ert: ert.enkf.enkf_main.EnKFMain """
        if self.__ert is not None:
            self.__ert.free()
            self.__ert = None

        self.__ert = ert

    def shell(self):
        """ @rtype: ErtShell """
        return self.__shell

    def __setitem__(self, key, value):
        self.__settings[key] = value

    def __getitem__(self, key):
        return self.__settings[key]