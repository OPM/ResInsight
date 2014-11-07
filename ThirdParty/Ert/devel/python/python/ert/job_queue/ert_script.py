import inspect
import imp
import sys
import traceback

class ErtScript(object):

    def __init__(self, ert):
        """
        @type ert: EnKFMain
        """
        super(ErtScript, self).__init__()

        if not hasattr(self, "run"):
            raise UserWarning("ErtScript implementations must provide a method run(self, ert, ...)")

        self.__verbose = False
        self.__ert = ert

        self.__is_cancelled = False

    def isVerbose(self):
        return self.__verbose

    def ert(self):
        """ @rtype: EnKFMain """
        return self.__ert

    def isCancelled(self):
        """ @rtype: bool """
        return self.__is_cancelled

    def cancel(self):
        self.__is_cancelled = True

    def initializeAndRun(self, argument_types, argument_values, verbose=False):
        """
        @type argument_types: list of type
        @type argument_values: list of string
        @type verbose: bool
        @rtype: unknown
        """
        self.__verbose = verbose

        arguments = []


        for index, arg_value in enumerate(argument_values):
            if index < len(argument_types):
                arg_type = argument_types[index]
            else:
                arg_type = str

            arguments.append(arg_type(arg_value))

        return self.run(*arguments)

    __module_count = 0 # Need to have unique modules in case of identical object naming in scripts

    @staticmethod
    def loadScriptFromFile(path):
        """ @rtype: type ErtScript """
        try:
            m = imp.load_source("ErtScriptModule_%d" % ErtScript.__module_count, path)
            ErtScript.__module_count += 1
            return ErtScript.__findErtScriptImplementations(m)
        except Exception as e:
            sys.stderr.write("The script '%s' caused an error during load:\n" % path)
            traceback.print_exception(sys.exc_type, sys.exc_value, None)
            return None

    @staticmethod
    def __findErtScriptImplementations(module):
        """ @rtype: ErtScript """
        result = []
        for name, obj in inspect.getmembers(module):
            if hasattr(obj, "__bases__") and ErtScript in obj.__bases__:
                result.append(obj)

        if len(result) != 1:
            raise UserWarning("Must have (only) one implementation of ErtScript in a module!")

        return result[0]