import re
from ert_gui.ide.keywords.definitions import ArgumentDefinition


class IntegerArgument(ArgumentDefinition):

    NOT_INTEGER = "The argument must be an integer."
    NOT_IN_RANGE = "The argument is not in range: %s"

    pattern = re.compile("^-?[0-9]+$")

    def __init__(self, from_value=None, to_value=None, **kwargs):
        super(IntegerArgument, self).__init__(**kwargs)
        self.from_value = from_value
        self.to_value = to_value


    def validate(self, token):
        validation_status = super(IntegerArgument, self).validate(token)

        match = IntegerArgument.pattern.match(token)

        if match is None:
            validation_status.setFailed()
            validation_status.addToMessage(IntegerArgument.NOT_INTEGER)
        else:
            value = int(token)

            if self.from_value is not None and self.to_value is not None and not self.from_value <= value <= self.to_value:
                validation_status.setFailed()
                range_string = "%d <= %d <= %d" % (self.from_value, value, self.to_value)
                validation_status.addToMessage(IntegerArgument.NOT_IN_RANGE % range_string)

            elif self.from_value is not None and self.from_value > value:
                validation_status.setFailed()
                range_string = "%d <= %d" % (self.from_value, value)
                validation_status.addToMessage(IntegerArgument.NOT_IN_RANGE % range_string)

            elif self.to_value is not None and self.to_value < value:
                validation_status.setFailed()
                range_string = "%d <= %d" % (value, self.to_value)
                validation_status.addToMessage(IntegerArgument.NOT_IN_RANGE % range_string)


            if not validation_status.failed():
                validation_status.setValue(value)


        return validation_status







