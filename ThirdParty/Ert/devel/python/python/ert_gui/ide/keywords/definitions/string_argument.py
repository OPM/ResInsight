import re
from ert_gui.ide.keywords.definitions import ArgumentDefinition


class StringArgument(ArgumentDefinition):

    NOT_A_VALID_STRING = "The argument must be a valid string."

    PATTERN = re.compile("^[\S]+$")
    PATTERN_WITH_SPACE = re.compile("^[\S| ]+$")


    def __init__(self, allow_space=False, **kwargs):
        super(StringArgument, self).__init__(**kwargs)
        self.__allow_space = allow_space


    def validate(self, token):
        validation_status = super(StringArgument, self).validate(token)

        if not validation_status:
            return validation_status
        elif self.isOptional() and token.strip() == "":
            return validation_status
        else:
            if self.__allow_space:
                match = StringArgument.PATTERN_WITH_SPACE.match(token)
            else:
                match = StringArgument.PATTERN.match(token)

            if match is None:
                validation_status.setFailed()
                validation_status.addToMessage(StringArgument.NOT_A_VALID_STRING)
            else:

                if not validation_status.failed():
                    validation_status.setValue(token)


            return validation_status







