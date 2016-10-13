
class ShellContext(object):
    def __init__(self, shell):
        super(ShellContext, self).__init__()

        self.__shell = shell
        """ :type: Cmd """

        self.__settings = {}


    def shell(self):
        """ @rtype: ert_gui.shell.ErtShell """
        return self.__shell

    def __setitem__(self, key, value):
        self.__settings[key] = value

    def __getitem__(self, key):
        return self.__settings[key]

    def __contains__(self, key):
        return key in self.__settings