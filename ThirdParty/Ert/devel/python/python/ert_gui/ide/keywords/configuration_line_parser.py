import re
from ert_gui.ide.keywords.data import Argument, Keyword


class ConfigurationLineParser(object):
    COMMENT_PATTERN = re.compile(".*?(--.*)")
    KEYWORD_PATTERN = re.compile("^\s*([A-Z_]+)(\s|--)?")
    ARGUMENT_PATTERN = re.compile('\s+?(\S+)\s*?')

    def __init__(self):
        super(ConfigurationLineParser, self).__init__()

        self.__comment_index = -1
        self.__keyword = None
        self.__text = None
        self.__arguments_index = -1
        self.__arguments = []


    def parseLine(self, line):
        self.__keyword = None
        self.__comment_index = -1
        self.__text = line
        self.__arguments_index = -1
        self.__arguments = []

        comment_match = re.match(ConfigurationLineParser.COMMENT_PATTERN, line)
        if comment_match is not None:
            self.__comment_index = comment_match.start(1)
            line = line[0:comment_match.start(1)]

        keyword_match = re.match(ConfigurationLineParser.KEYWORD_PATTERN, line)
        if keyword_match is not None:
            self.__keyword = Keyword(keyword_match.start(1), keyword_match.end(1), line)
            self.__arguments_index = keyword_match.end(1)

        if self.hasKeyword():
            argument_match = ConfigurationLineParser.ARGUMENT_PATTERN.finditer(line)

            for match in argument_match:
                self.__arguments.append(Argument(match.start(1), match.end(1), line))


    def hasComment(self):
        """ @rtype: bool """
        return self.__comment_index >= 0

    def commentIndex(self):
        """ @rtype: str """
        return self.__comment_index

    def hasKeyword(self):
        """ @rtype: bool """
        return self.__keyword is not None

    def keyword(self):
        """ @rtype: Keyword """
        return self.__keyword

    def text(self):
        """ @rtype: str """
        return self.__text

    def uncommentedText(self):
        """ @rtype: str """
        if self.hasComment():
            return self.__text[0:self.commentIndex()]
        return self.text()

    def argumentsText(self):
        """ @rtype: str """
        if self.hasKeyword():
            return self.uncommentedText()[self.__arguments_index:]
        return ""

    def arguments(self):
        """ @rtype: list of Argument """
        return self.__arguments


