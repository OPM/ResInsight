import re
from ert_gui.ide.keywords.definitions import ArgumentDefinition


class NumberListStringArgument(ArgumentDefinition):

    NOT_A_VALID_NUMBER_LIST_STRING = "The input should be of the type: <b><pre>\n\t23,5.5,11,1.01,3\n</pre></b>i.e. numeric values separated by commas."
    VALUE_NOT_A_NUMBER = "The value: '%s' is not a number."

    PATTERN = re.compile("^[0-9\.\-+, \t]+$")

    def __init__(self, **kwargs):
        super(NumberListStringArgument, self).__init__(**kwargs)

    def validate(self, token):
        validation_status = super(NumberListStringArgument, self).validate(token)

        if not validation_status:
            return validation_status
        else:
            match = NumberListStringArgument.PATTERN.match(token)

            if match is None:
                validation_status.setFailed()
                validation_status.addToMessage(NumberListStringArgument.NOT_A_VALID_NUMBER_LIST_STRING)
            else:

                groups = token.split(",")

                for group in groups:
                    group = group.strip()

                    if len(group) > 0:
                        try:
                            num = float(group.strip())
                        except ValueError:
                            validation_status.setFailed()
                            validation_status.addToMessage(NumberListStringArgument.VALUE_NOT_A_NUMBER % group)


                validation_status.setValue(token)

            return validation_status
