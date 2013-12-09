import re
from ert_gui.ide.keywords.definitions import ArgumentDefinition


class ProperNameFormatArgument(ArgumentDefinition):

    NOT_A_VALID_NAME_FORMAT = "The argument must be a valid string containing a %d and only characters of these types:" \
                       "<ul>" \
                       "<li>Letters: <code>A-Z</code> and <code>a-z</code></li>" \
                       "<li>Numbers: <code>0-9</code></li>" \
                       "<li>Underscore: <code>_</code></li>" \
                       "<li>Dash: <code>&mdash;</code><li>" \
                       "<li>Period: <code>.</code></li>" \
                       "<li>Brackets: </code>&lt;&gt;</code></li>" \
                       "</ul>"


    PATTERN = re.compile("^[A-Za-z0-9_\-.<>]*(%d)[A-Za-z0-9_\-.<>]*$")


    def __init__(self, **kwargs):
        super(ProperNameFormatArgument, self).__init__(**kwargs)


    def validate(self, token):
        validation_status = super(ProperNameFormatArgument, self).validate(token)

        if not validation_status:
            return validation_status
        else:
            match = ProperNameFormatArgument.PATTERN.match(token)

            if match is None:
                validation_status.setFailed()
                validation_status.addToMessage(ProperNameFormatArgument.NOT_A_VALID_NAME_FORMAT)
            else:

                if not validation_status.failed():
                    validation_status.setValue(token)

            return validation_status







