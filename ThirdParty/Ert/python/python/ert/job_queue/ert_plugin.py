from .ert_script import ErtScript
from threading import Thread
import time

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

    def checkIfCancelled(self):
        if self.isCancelled():
            raise CancelPluginException("Plugin '%s' cancelled by user!" % self.getName())

    def startCancellableThread(self, runFunction, cancelFunction):
        runFunction.return_value = None

        def runFunctionWrapper():
            runFunction.return_value = runFunction()

        thread = Thread()
        thread.run = runFunctionWrapper
        thread.start()

        while thread.isAlive():
            if self.isCancelled():
                cancelFunction()

            try:
                time.sleep(0.1)
            except KeyboardInterrupt:
                print("Plugin '%s' cancelled (CTRL+C)" % self.getName())
                self.cancel()

        return runFunction.return_value
