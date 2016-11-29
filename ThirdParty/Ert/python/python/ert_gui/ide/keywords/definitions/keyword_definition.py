class KeywordDefinition(object):

    def __init__(self, name):
        super(KeywordDefinition, self).__init__()
        self.__name = name

    def name(self):
        return self.__name

