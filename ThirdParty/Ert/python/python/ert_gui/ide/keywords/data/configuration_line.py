from ert_gui.ide.keywords.data import Argument, Keyword, ValidationStatus
from ert_gui.ide.keywords.definitions import ArgumentDefinition, KeywordDefinition


class ConfigurationLine(object):
    ARGUMENT_NOT_EXPECTED = "Argument not expected!"
    ARGUMENT_ERROR = "Keyword has an argument error!"
    UNKNOWN_KEYWORD = "Unknown keyword!"

    def __init__(self, keyword, arguments, documentation_link, group, required=False):
        """
         @type keyword: Keyword
         @type arguments: list of Argument
         @type documentation_link: str
         @type group: str
         @type required: bool
        """
        super(ConfigurationLine, self).__init__()

        self.__keyword = keyword
        self.__arguments = arguments
        self.__documentation_link = documentation_link
        self.__required = required
        self.__group = group
        self.__validation_status = {}

        self.__validateTokens()


    def __validateTokens(self):
        keyword_validation_status = ValidationStatus()

        if not self.__keyword.hasKeywordDefinition():
            keyword_validation_status.setFailed()
            keyword_validation_status.addToMessage(ConfigurationLine.UNKNOWN_KEYWORD)

        self.__validation_status[self.__keyword] = keyword_validation_status

        argument_error = False
        for argument in self.__arguments:
            argument_validation_status = ValidationStatus()

            if not argument.hasArgumentDefinition():
                argument_validation_status.setFailed()
                argument_validation_status.addToMessage(ConfigurationLine.ARGUMENT_NOT_EXPECTED)

                argument_error = True
            else:
                arg_def = argument.argumentDefinition()
                argument_validation_status = arg_def.validate(argument.value())

                if not argument_validation_status:
                    argument_error = True

            self.__validation_status[argument] = argument_validation_status

        if argument_error:
            keyword_validation_status.setFailed()
            keyword_validation_status.addToMessage(ConfigurationLine.ARGUMENT_ERROR)

            for argument in self.__arguments:
                argument_validation_status = self.validationStatusForToken(argument)
                if not argument_validation_status:
                    keyword_validation_status.addToMessage(argument_validation_status.message())


    def keyword(self):
        """ @rtype: Keyword"""
        return self.__keyword

    def arguments(self):
        """ @rtype: list of Argument """
        return self.__arguments

    def isRequired(self):
        """ @rtype: bool """
        return self.__required

    def documentationLink(self):
        """ @rtype: str """
        return self.__documentation_link

    def group(self):
        """ @rtype: str """
        return self.__group

    def validationStatusForToken(self, token):
        """ @rtype: ValidationStatus """
        return self.__validation_status[token]
