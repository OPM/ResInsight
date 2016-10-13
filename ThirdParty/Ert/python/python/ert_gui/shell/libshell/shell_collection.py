import shlex
import textwrap
from ert_gui.shell.libshell import autoCompleteList, ShellFunction, ShellProperty, widthAsPercentageOfConsoleWidth, getTerminalSize


class ShellCollection(object):
    command_help_message = "The command: '%s' supports the following keywords:"

    def __init__(self, name, parent=None, description="No description available"):
        super(ShellCollection, self).__init__()
        self.__name = name
        self.__parent = None
        self.__description = description

        if parent is not None:
            self.setParent(parent)
            parent.addChild(self)

        self.__collection = {}
        self.__model_tracker = {}
        self.__children = []

    def setParent(self, parent):
        if not hasattr(parent, "shellContext"):
            raise ValueError("Parent is missing function: shellContext()")

        if not hasattr(parent, "lastCommandFailed"):
            raise ValueError("Parent is missing function: lastCommandFailed()")

        setattr(parent, "do_%s" % self.name, self.doKeywords)
        setattr(parent, "complete_%s" % self.name, self.completeKeywords)
        setattr(parent, "help_%s" % self.name, self.helpKeywords)
        self.__parent = parent


    def addChild(self, child):
        self.__children.append(child)

    def cleanup(self):
        for child in self.__children:
            child.cleanup()

    def addCollection(self, collection):
        """
        :type collection: ShellCollection
        """
        self.__collection[collection.name] = collection
        collection.setParent(self)


    def addProperty(self, property):
        """
        :type property: ShellProperty
        """
        self.__collection[property.name] = property
        property.setParent(self)

    def addFunction(self, function):
        """
        :type function: ShellFunction
        """
        self.__collection[function.name] = function
        function.setParent(self)


    def addShellProperty(self, name, getter, setter=None, validator=None, completer=None, help_arguments=None, help_message=None, pretty_attribute=None, model=None):
        """ @rtype: ShellProperty """
        shell_property = ShellProperty(name, getter, setter, validator, completer, help_arguments, help_message, pretty_attribute)
        self.addProperty(shell_property)

        if model is None:
            model = self

        self.__model_tracker[name] = model
        return shell_property


    def getModelForProperty(self, property_name):
        return self.__model_tracker[property_name]


    def addShellFunction(self, name, function, completer=None, help_arguments=None, help_message=None, model=None):
        """ @rtype: ShellFunction """
        func = ShellFunction(name, function, completer, help_arguments, help_message)
        self.addFunction(func)

        if model is None:
            model = self

        self.__model_tracker[name] = model

        return func


    def getModelForFunction(self, name):
        return self.__model_tracker[name]

    @property
    def name(self):
        return self.__name

    def shellContext(self):
        """ :rtype: ert_gui.shell.libshell.ShellContext """
        return self.__parent.shellContext()

    def lastCommandFailed(self, message):
        self.__parent.lastCommandFailed(message)

    def findKeywords(self):
        return self.__collection.keys()

    def completeKeywords(self, text, line, begidx, endidx):
        arguments = shlex.split(line)
        assert arguments[0] == self.name

        line = line[len(self.name) + 1:]
        begidx = begidx - len(self.name) + 1
        endidx = endidx - len(self.name) + 1
        keyword, sep, arguments = line.partition(' ')

        if begidx >= len(keyword) and keyword in self.findKeywords():
            if hasattr(self, "complete_%s" % keyword):
                func = getattr(self, "complete_%s" % keyword)
                return func(text, line, begidx, endidx)
            else:
                return []
        else:
            return autoCompleteList(text, self.findKeywords())

    def doKeywords(self, line):
        keyword, sep, arguments = line.partition(' ')

        if keyword.strip() == "":
            self.printGuidance()
        elif keyword in self.__collection:
            func = getattr(self, "do_%s" % keyword)
            return func(arguments)
        else:
            self.lastCommandFailed("Unknown keyword: '%s'" % keyword)
            self.printGuidance()


    def printGuidance(self):
        print(self.command_help_message % self.name)
        self.shellContext().shell().columnize(self.findKeywords(), getTerminalSize()[0])


    def helpKeywords(self):
        print(self.command_help_message % self.name)
        keywords = self.findKeywords()

        keyword_column_width = widthAsPercentageOfConsoleWidth(20)
        parameter_column_width = widthAsPercentageOfConsoleWidth(30)
        help_column_width = widthAsPercentageOfConsoleWidth(48)
        help_format = " %-" + str(keyword_column_width) + "s %-" + str(parameter_column_width) + "s %-" + str(help_column_width) + "s"

        print(help_format % ("Keyword", "Parameter(s)", "Help"))

        for keyword in keywords:
            message = "No help available!"
            parameters = None
            if hasattr(self, "help_tuple_%s" % keyword):
                func = getattr(self, "help_tuple_%s" % keyword)
                _, parameters, message = func()

            message = textwrap.wrap(message, help_column_width)
            print(help_format % (keyword, parameters, message[0]))

            if len(message) > 1:
                for line in message[1:]:
                    print(help_format % ("", "", line))