
import re
from ert_gui.ide.keywords.definitions import ArgumentDefinition


class BoolArgument(ArgumentDefinition):

    NOT_BOOL = "The argument must be TRUE or FALSE."

    true_pattern  = re.compile("^(1|T|[Tt][Rr][Uu][Ee])$")
    false_pattern  = re.compile("^(0|F|[Ff][Aa][Ll][Ss][Ee])$")

    def __init__(self, **kwargs):
        super(BoolArgument, self).__init__(**kwargs)


    def validate(self, token):
        validation_status = super(BoolArgument, self).validate(token)

        true_match = BoolArgument.true_pattern.match(token)
        false_match = BoolArgument.false_pattern.match(token)

        if true_match is None and false_match is None:
            validation_status.setFailed()
            validation_status.addToMessage(BoolArgument.NOT_BOOL)
        else:
            validation_status.setValue(true_match is not None)

        return validation_status







