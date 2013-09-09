class CNamespace(object):
    def __init__(self, name):
        object.__setattr__(self, "_name", name)
        object.__setattr__(self, "_functions", {})

    def __str__(self):
        return "%s wrapper" % self._name

    def __setitem__(self, key, value):
        self.__setValue(key, value)


    def __getitem__(self, item):
        return self._functions[item]

    def __setattr__(self, key, value):
        self.__setValue(key, value)

    def __setValue(self, key, value):
        assert not hasattr(self, key), "The namespace %s already contains a function named %s!" % (self._name, key)

        self._functions[key] = value
        object.__setattr__(self, key, value)


