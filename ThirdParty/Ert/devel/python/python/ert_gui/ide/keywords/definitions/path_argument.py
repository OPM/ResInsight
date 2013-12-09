import os
import re
from ert_gui.ide.keywords.definitions import ArgumentDefinition


class PathArgument(ArgumentDefinition):

    NOT_A_VALID_PATH = "The argument must be a valid path."
    PATH_DOES_NOT_EXIST = "The argument must be a valid path that exists."

    PATTERN = re.compile("^[\S]+$")
    PATTERN_WITH_SPACE = re.compile("^[\S| ]+$")


    def __init__(self, must_exist=True, **kwargs):
        super(PathArgument, self).__init__(**kwargs)
        self.__must_exist = must_exist


    def validate(self, token):
        validation_status = super(PathArgument, self).validate(token)

        if not os.path.exists(token):
            validation_status.setFailed()
            validation_status.addToMessage(PathArgument.PATH_DOES_NOT_EXIST)

        return validation_status







