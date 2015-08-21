from ert_gui.models.mixins import PathModelMixin


class DefaultPathModel (PathModelMixin):
    def __init__(self, default_path, is_required=True, must_be_a_directory=False, must_be_a_file=True, must_exist=False, must_be_absolute=False, must_be_executable=False):
        self.__path = default_path
        super(DefaultPathModel, self).__init__()

        self.path_is_required = is_required
        self.path_must_be_a_directory = must_be_a_directory
        self.path_must_be_a_file = must_be_a_file
        self.path_must_be_executable = must_be_executable
        self.path_must_exist = must_exist
        self.path_must_be_absolute = must_be_absolute


    def pathIsRequired(self):
        """ @rtype: bool """
        return self.path_is_required

    def pathMustBeADirectory(self):
        """ @rtype: bool """
        return self.path_must_be_a_directory

    def pathMustBeAFile(self):
        """ @rtype: bool """
        return self.path_must_be_a_file

    def pathMustBeExecutable(self):
        """ @rtype: bool """
        return self.path_must_be_executable

    def pathMustExist(self):
        """ @rtype: bool """
        return self.path_must_exist

    def pathMustBeAbsolute(self):
        """ @rtype: bool """
        return self.path_must_be_absolute

    def getPath(self):
        """ @rtype: str """
        return self.__path

    def setPath(self, value):
        """
        @type value: str
        """
        self.__path = value
        self.observable().notify(self.PATH_CHANGED_EVENT)