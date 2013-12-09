from ert_gui.ide.keywords import ErtKeywords
from ert_gui.ide.keywords.configuration_line_parser import ConfigurationLineParser
from ert_gui.ide.keywords.data import ConfigurationLine, Argument, Keyword


class ConfigurationLineBuilder(object):
    DEFAULT_GROUP = "Unknown keyword"
    DEFAULT_DOCUMENTATION_LINK = "unknown_keyword"

    def __init__(self, keywords):
        super(ConfigurationLineBuilder, self).__init__()

        assert isinstance(keywords, ErtKeywords)
        self.__keywords = keywords
        self.__configuration_line_parser = ConfigurationLineParser()
        self.__configuration_line = None


    def processLine(self, line):
        self.__configuration_line_parser.parseLine(line)
        self.__configuration_line = None

        if self.__configuration_line_parser.hasKeyword():
            keyword = self.__configuration_line_parser.keyword()
            arguments = self.__configuration_line_parser.arguments()

            documentation_link = ConfigurationLineBuilder.DEFAULT_DOCUMENTATION_LINK
            group = ConfigurationLineBuilder.DEFAULT_GROUP
            required = False

            if keyword.value() in self.__keywords:
                configuration_line_definition = self.__keywords[keyword.value()]

                documentation_link = configuration_line_definition.documentationLink()
                group = configuration_line_definition.group()
                required = configuration_line_definition.isRequired()

                keyword.setKeywordDefinition(configuration_line_definition.keywordDefinition())

                arguments = self.__matchArguments(keyword, configuration_line_definition.argumentDefinitions(), arguments)


            self.__configuration_line = ConfigurationLine(keyword, arguments, documentation_link, group, required)

    def configurationLine(self):
        """ @rtype: ConfigurationLine """
        return self.__configuration_line

    def hasConfigurationLine(self):
        """ @rtype: bool """
        return self.__configuration_line is not None

    def hasComment(self):
        """ @rtype: bool """
        return self.__configuration_line_parser.hasComment()

    def commentIndex(self):
        return self.__configuration_line_parser.commentIndex()


    def __matchArguments(self, keyword, arg_defs, args):
        """
         @type keyword: Keyword
         @type arg_defs: list of ArgumentDefinition
         @type args: list of Argument
         @rtype: list of Argument
        """
        arg_def_count = len(arg_defs)
        arg_count = len(args)

        if arg_count > arg_def_count:
            # merge last input arguments

            last_arg_def = arg_defs[len(arg_defs) - 1]

            if last_arg_def.consumeRestOfLine():
                from_arg = args[arg_def_count - 1]
                to_arg = args[arg_count - 1]

                last_argument = Argument(from_arg.fromIndex(), to_arg.toIndex(), keyword.line())
                args = args[0:arg_def_count]
                args[len(args) - 1] = last_argument

            else:
                from_arg = args[arg_def_count]
                to_arg = args[arg_count - 1]

                last_argument = Argument(from_arg.fromIndex(), to_arg.toIndex(), keyword.line())
                args = args[0:arg_def_count]
                args.append(last_argument)

        if arg_count < arg_def_count:
            # pad with empty arguments
            line = keyword.line()

            for index in range(arg_def_count - arg_count):
                empty_argument = Argument(len(line), len(line), line)
                args.append(empty_argument)


        for index in range(len(args)):
            if index < len(arg_defs):
                args[index].setArgumentDefinition(arg_defs[index])


        return args




