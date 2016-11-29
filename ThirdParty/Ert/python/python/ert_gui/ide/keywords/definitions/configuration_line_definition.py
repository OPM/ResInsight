from ert_gui.ide.keywords.definitions import KeywordDefinition, ArgumentDefinition


class ConfigurationLineDefinition(object):
    def __init__(self, keyword, arguments, documentation_link, group, required=False):
        super(ConfigurationLineDefinition, self).__init__()

        self.__keyword_definition = keyword
        self.__argument_definitions = arguments
        self.__documentation_link = documentation_link
        self.__required = required
        self.__group = group

    def isRequired(self):
        """ @rtype: bool """
        return self.__required

    def keywordDefinition(self):
        """ @rtype: KeywordDefinition """
        return self.__keyword_definition

    def argumentDefinitions(self):
        """ @rtype: list of ArgumentDefinition """
        return self.__argument_definitions

    def documentationLink(self):
        """ @rtype: str """
        return self.__documentation_link

    def group(self):
        """ @rtype: str """
        return self.__group