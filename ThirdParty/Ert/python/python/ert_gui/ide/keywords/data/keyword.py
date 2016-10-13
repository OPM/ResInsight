from ert_gui.ide.keywords.data import Token
from ert_gui.ide.keywords.definitions import KeywordDefinition


class Keyword(Token):
    def __init__(self, from_index, to_index, line):
        super(Keyword, self).__init__(from_index, to_index, line)

        self.__keyword_definition = None

    def keywordDefinition(self):
        """ @rtype: KeywordDefinition """
        return self.__keyword_definition

    def setKeywordDefinition(self, keyword_definition):
        self.__keyword_definition = keyword_definition

    def hasKeywordDefinition(self):
        return self.__keyword_definition is not None