from .ert_script import ErtScript


class CancelPluginException(Exception):
    def __init__(self, cancel_message):
        super(CancelPluginException, self).__init__(cancel_message)


class ErtPlugin(ErtScript):

    def getArguments(self, parent=None):
        """ @rtype: list """
        return []

    def getName(self):
        """ @rtype: str """
        return str(self.__class__)

    def getDescription(self):
        """ @rtype: str """
        return "No description provided!"