from ert_gui.shell.libshell import extractFullArgument, autoCompleteListWithSeparator

class ShellProperty(object):
    def __init__(self, name, getter, setter=None, validator=None, completer=None, help_arguments=None, help_message=None, pretty_attribute=None):
        super(ShellProperty, self).__init__()
        self.__parent = None

        if getter is None:
            raise ValueError("Getter function/property can not be None for shell property '%s'" % name)

        self.__setter = setter
        self.__getter = getter

        self.__name = name
        self.__pretty_attribute = name if pretty_attribute is None else pretty_attribute

        self.__help_arguments = help_arguments
        self.__help_message = help_message if help_message is not None else "No help available!"

        self.__completer = completer
        self.__validator = validator



    def setParent(self, parent):
        if parent is None:
            raise ValueError("Property target can not be None for shell property '%s'" % self.name)

        if not hasattr(parent, "getModelForProperty"):
            raise ValueError("Property target is missing method 'getModelForProperty(name)' for shell property '%s'" % self.name)

        if hasattr(parent, "do_%s" % self.name) or hasattr(parent, "complete_%s" % self.name) or hasattr(parent, "help_%s" % self.name) :
            raise ValueError("Property with name '%s' already exists for object '%s'" % (self.name, parent))

        setattr(parent, "do_%s" % self.name, self.doFunction)
        setattr(parent, "complete_%s" % self.name, self.completeFunction)
        setattr(parent, "help_tuple_%s" % self.name, self.helpFunction)

        self.__parent = parent


    def doFunction(self, line):
        value = line.strip()
        model = self.getModelForProperty()
        if value == "":
            if isinstance(self.__getter, property):
                result = self.__getter.__get__(model)
            else:
                result = self.__getter(model)
            print("%s = %s" % (self.__pretty_attribute, result))
        else:
            try:
                if self.__validator is not None:
                    value = self.__validator(model, value)

                if self.__setter is not None:
                    if isinstance(self.__setter, property):
                        self.__setter.__set__(model, value)
                    else:
                        self.__setter(model, value)
                    print("%s set to: %s" % (self.__pretty_attribute, value))
                else:
                    self.lastCommandFailed("Property '%s' is set to read only" % self.__pretty_attribute)
            except ValueError as e:
                self.lastCommandFailed(e.message)


    def completeFunction(self, text, line, begidx, endidx):
        if self.__completer is not None:
            if callable(self.__completer):
                model = self.getModelForProperty()
                return self.__completer(model, text, line, begidx, endidx)
            else:
                key = extractFullArgument(line, endidx)
                return autoCompleteListWithSeparator(key, self.__completer)

        return []

    def helpFunction(self):
        return self.name, self.__help_arguments, self.__help_message

    def getModelForProperty(self):
        return self.__parent.getModelForProperty(self.name)

    def lastCommandFailed(self, message):
        self.__parent.lastCommandFailed(message)

    @property
    def name(self):
        return self.__name
