from ert_gui.shell.libshell import extractFullArgument, autoCompleteListWithSeparator


class ShellFunction(object):

    def __init__(self, name, function, completer=None, help_arguments=None, help_message=None):
        super(ShellFunction, self).__init__()
        self.__parent = None
        self.__name = name

        if function is None:
            raise ValueError("Function can not be None for shell function '%s'" % name)

        self.__function = function
        self.__completer = completer

        self.__help_arguments = help_arguments
        self.__help_message = help_message if help_message is not None else "No help available!"


    def setParent(self, parent):
        if parent is None:
            raise ValueError("Function target can not be None for shell function '%s'" % self.name)

        if not hasattr(parent, "getModelForFunction"):
            raise ValueError("function target is missing method 'getModelForFunction(name)' for shell function '%s'" % self.name)

        if hasattr(parent, "do_%s" % self.name) or hasattr(parent, "complete_%s" % self.name) or hasattr(parent, "help_%s" % self.name) :
            raise ValueError("function with name '%s' already exists for object '%s'" % (self.name, parent))

        setattr(parent, "do_%s" % self.name, self.doFunction)
        setattr(parent, "complete_%s" % self.name, self.completeFunction)
        setattr(parent, "help_tuple_%s" % self.name, self.helpFunction)

        self.__parent = parent


    def doFunction(self, line):
        model = self.getModelForFunction()
        self.__function(model, line.strip())

    def completeFunction(self, text, line, begidx, endidx):
        if self.__completer is not None:
            if callable(self.__completer):
                model = self.getModelForFunction()
                return self.__completer(model, text, line, begidx, endidx)
            else:
                key = extractFullArgument(line, endidx)
                return autoCompleteListWithSeparator(key, self.__completer)

        return []

    def helpFunction(self):
        return self.name, self.__help_arguments, self.__help_message

    def getModelForFunction(self):
        return self.__parent.getModelForFunction(self.name)

    @property
    def name(self):
        return self.__name

