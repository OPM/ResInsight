from ert_gui.ertwidgets.models.valuemodel import ValueModel


class PathModel(ValueModel):
    def __init__(self, default_path, is_required=True, must_be_a_directory=False, must_be_a_file=True,
                 must_exist=False, must_be_absolute=False, must_be_executable=False):
        ValueModel.__init__(self, default_path)

        self._path_is_required = is_required
        self._path_must_be_a_directory = must_be_a_directory
        self._path_must_be_a_file = must_be_a_file
        self._path_must_be_executable = must_be_executable
        self._path_must_exist = must_exist
        self._path_must_be_absolute = must_be_absolute

    def pathIsRequired(self):
        """ @rtype: bool """
        return self._path_is_required

    def pathMustBeADirectory(self):
        """ @rtype: bool """
        return self._path_must_be_a_directory

    def pathMustBeAFile(self):
        """ @rtype: bool """
        return self._path_must_be_a_file

    def pathMustBeExecutable(self):
        """ @rtype: bool """
        return self._path_must_be_executable

    def pathMustExist(self):
        """ @rtype: bool """
        return self._path_must_exist

    def pathMustBeAbsolute(self):
        """ @rtype: bool """
        return self._path_must_be_absolute

    def getPath(self):
        """ @rtype: str """
        return self.getValue()

    def setPath(self, value):
        """
        @type value: str
        """
        self.setValue(value)
