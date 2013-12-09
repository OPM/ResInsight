import re
from ert_gui.ide.keywords.definitions import ArgumentDefinition


class FloatArgument(ArgumentDefinition):

    NOT_FLOAT = "The argument must be a float."
    NOT_IN_RANGE = "The argument is not in range: %s"

    pattern  = re.compile("^[\S]+$")

    def __init__(self, from_value=None, to_value=None, **kwargs):
        super(FloatArgument, self).__init__(**kwargs)
        self.from_value = from_value
        self.to_value = to_value


    def validate(self, token):
        validation_status = super(FloatArgument, self).validate(token)

        match = FloatArgument.pattern.match(token)

        if match is None:
            validation_status.setFailed()
            validation_status.addToMessage(FloatArgument.NOT_FLOAT)
        else:
            try:
                value = float(token)

                if self.from_value is not None and self.to_value is not None and not self.from_value <= value <= self.to_value:
                    validation_status.setFailed()
                    range_string = "%f <= %f <= %f" % (self.from_value, value, self.to_value)
                    validation_status.addToMessage(FloatArgument.NOT_IN_RANGE % range_string)

                elif self.from_value is not None and self.from_value > value:
                    validation_status.setFailed()
                    range_string = "%f <= %f" % (self.from_value, value)
                    validation_status.addToMessage(FloatArgument.NOT_IN_RANGE % range_string)

                elif self.to_value is not None and self.to_value < value:
                    validation_status.setFailed()
                    range_string = "%f <= %f" % (value, self.to_value)
                    validation_status.addToMessage(FloatArgument.NOT_IN_RANGE % range_string)

                if not validation_status.failed():
                    validation_status.setValue(value)

            except ValueError:
                validation_status.setFailed()
                validation_status.addToMessage(FloatArgument.NOT_FLOAT)

        return validation_status







