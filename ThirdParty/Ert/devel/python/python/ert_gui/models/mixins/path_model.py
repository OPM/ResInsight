from ert_gui.models.mixins import ModelMixin, AbstractMethodError


class PathModelMixin(ModelMixin):
    PATH_CHANGED_EVENT = "path_changed_event"

    def registerDefaultEvents(self):
        super(PathModelMixin, self).registerDefaultEvents()
        self.observable().addEvent(PathModelMixin.PATH_CHANGED_EVENT)

    def pathIsRequired(self):
        """ @rtype: bool """
        raise AbstractMethodError(self,  "isRequired")

    def pathMustExist(self):
        """ @rtype: bool """
        raise AbstractMethodError(self, "pathMustExist")

    def pathMustBeExecutable(self):
        """ @rtype: bool """
        raise AbstractMethodError(self, "pathMustBeExecutable")

    def pathMustBeAFile(self):
        """ @rtype: bool """
        raise AbstractMethodError(self, "pathMustBeAFile")

    def pathMustBeADirectory(self):
        """ @rtype: bool """
        raise AbstractMethodError(self, "pathMustBeADirectory")

    def pathMustBeAbsolute(self):
        """ @rtype: bool """
        raise AbstractMethodError(self, "pathMustBeExecutable")

    def getPath(self):
        """ @rtype: str """
        raise AbstractMethodError(self, "getValue")

    def setPath(self, value):
        raise AbstractMethodError(self, "setValue")


