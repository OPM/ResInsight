class DictProperty(object):
    def __init__(self, name):
        super(DictProperty, self).__init__()
        self.__property_name = name

    def __set__(self, instance, value):
        instance[self.__property_name] = value

    def __get__(self, instance, owner):
        if not self.__property_name in instance:
            raise AttributeError("The dictionary property: '%s' has not been initialized!" % self.__property_name)

        if not owner.__dict__.has_key(self.__property_name):
            raise AttributeError("The dictionary property: '%s' does not have an associated attribute!" % self.__property_name)

        return instance[self.__property_name]