import re
from ert_gui.ide.keywords.definitions import ArgumentDefinition


class PercentArgument(ArgumentDefinition):

    NOT_PERCENT = "The argument must be a number followed by % - no space allowed."
    NOT_IN_RANGE = "The argument is not in range: %s"

    pattern = re.compile("^-?[0-9]+(\.[0-9]+)?\%$")

    def __init__(self, from_value=None, to_value=None, **kwargs):
        super(PercentArgument, self).__init__(**kwargs)
        self.from_value = from_value * 0.01
        self.to_value = to_value  * 0.01


    def validate(self, token):
        validation_status = super(PercentArgument, self).validate(token)

        match = PercentArgument.pattern.match(token)

        if match is None:
            validation_status.setFailed()
            validation_status.addToMessage(PercentArgument.NOT_PERCENT)
        else:
            value = float(token[:-1]) * 0.01

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







