class Token(object):

    def __init__(self, from_index, to_index, line):
        super(Token, self).__init__()

        self.__from_index = from_index
        self.__to_index = to_index
        self.__line = line

    def value(self):
        """ @rtype: str """
        return self.__line[self.__from_index:self.__to_index]

    def fromIndex(self):
        """ @rtype: int """
        return self.__from_index

    def toIndex(self):
        """ @rtype: int """
        return self.__to_index

    def line(self):
        """ @rtype: str """
        return self.__line

    def count(self):
        """ @rtype: int """
        return self.toIndex() - self.fromIndex()

    def __contains__(self, item):
        assert isinstance(item, int)

        return self.__from_index <= item < self.__to_index



