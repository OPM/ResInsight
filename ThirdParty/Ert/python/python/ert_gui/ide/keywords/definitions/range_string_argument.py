import re
from ert_gui.ide.keywords.definitions import ArgumentDefinition


class RangeStringArgument(ArgumentDefinition):

    NOT_A_VALID_RANGE_STRING = "The input should be of the type: <b><pre>\n\t1,3-5,9,17\n</pre></b>i.e. integer values separated by commas, and dashes to represent ranges."
    VALUE_NOT_IN_RANGE = "A value must be in the range from 0 to %d."


    PATTERN = re.compile("^[0-9\-, \t]+$")
    RANGE_PATTERN = re.compile("^[ \t]*([0-9]+)[ \t]*-[ \t]*([0-9]+)[ \t]*$")
    NUMBER_PATTERN = re.compile("^[ \t]*([0-9]+)[ \t]*$")


    def __init__(self, max_value=None, **kwargs):
        super(RangeStringArgument, self).__init__(**kwargs)
        self.__max_value = max_value

    def validate(self, token):
        validation_status = super(RangeStringArgument, self).validate(token)

        if not validation_status:
            return validation_status
        else:
            match = RangeStringArgument.PATTERN.match(token)

            if match is None:
                validation_status.setFailed()
                validation_status.addToMessage(RangeStringArgument.NOT_A_VALID_RANGE_STRING)
            else:

                groups = token.split(",")

                for group in groups:
                    range_match = RangeStringArgument.RANGE_PATTERN.match(group)
                    number_match = RangeStringArgument.NUMBER_PATTERN.match(group)


                    if range_match is None and number_match is None:
                        validation_status.setFailed()
                        validation_status.addToMessage(RangeStringArgument.NOT_A_VALID_RANGE_STRING)
                        break

                    if range_match:
                        num_1 = int(range_match.group(1))
                        num_2 = int(range_match.group(2))

                        if not num_2 > num_1:
                            validation_status.setFailed()
                            validation_status.addToMessage(RangeStringArgument.NOT_A_VALID_RANGE_STRING)
                            break

                        if self.__max_value is not None and (num_1 >= self.__max_value or num_2 >= self.__max_value):
                            validation_status.setFailed()
                            validation_status.addToMessage(RangeStringArgument.VALUE_NOT_IN_RANGE % (self.__max_value - 1))
                            break

                    if number_match and self.__max_value is not None:
                        num = int(number_match.group(1))

                        if num >= self.__max_value:
                            validation_status.setFailed()
                            validation_status.addToMessage(RangeStringArgument.VALUE_NOT_IN_RANGE % (self.__max_value - 1))
                            break



                validation_status.setValue(token)

            return validation_status
