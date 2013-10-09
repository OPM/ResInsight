from ert_gui.models import ErtConnector
from ert_gui.models.mixins import PathModelMixin


class RshCommand(ErtConnector, PathModelMixin):

    def pathIsRequired(self):
        return True

    def pathMustBeADirectory(self):
        return False

    def pathMustBeAFile(self):
        return True

    def pathMustBeExecutable(self):
        return True

    def pathMustExist(self):
        return True

    def pathMustBeAbsolute(self):
        return True

    def getPath(self):
        return self.ert().siteConfig().getRshCommand()

    def setPath(self, value):
        self.ert().siteConfig().set_rsh_command(str(value))
        self.observable().notify(self.PATH_CHANGED_EVENT)




