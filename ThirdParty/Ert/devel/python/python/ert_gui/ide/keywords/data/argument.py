from ert_gui.ide.keywords.data import Token


class Argument(Token):
    def __init__(self, from_index, to_index, line):
        super(Argument, self).__init__(from_index, to_index, line)
        self.__argument_definition = None


    def setArgumentDefinition(self, argument_definition):
        self.__argument_definition = argument_definition

    def argumentDefinition(self):
        """ @rtype: ArgumentDefinition """
        return self.__argument_definition

    def hasArgumentDefinition(self):
        """ @rtype: bool """
        return self.__argument_definition is not None








