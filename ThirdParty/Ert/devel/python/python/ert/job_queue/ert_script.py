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
        self.__failed = False

    def isVerbose(self):
        return self.__verbose

    def ert(self):
        """ @rtype: ert.enkf.EnKFMain """
        return self.__ert

    def isCancelled(self):
        """ @rtype: bool """
        return self.__is_cancelled

    def hasFailed(self):
        """ @rtype: bool """
        return self.__failed

    def cancel(self):
        self.__is_cancelled = True

    def cleanup(self):
        """
        Override to perform cleanup after a run.
        """
        pass

    def initializeAndRun(self, argument_types, argument_values, verbose=False):
        """
        @type argument_types: list of type
        @type argument_values: list of string
        @type verbose: bool
        @rtype: unknown
        """
        self.__verbose = verbose
        self.__failed = False

        arguments = []


        for index, arg_value in enumerate(argument_values):
            if index < len(argument_types):
                arg_type = argument_types[index]
            else:
                arg_type = str

            if arg_value is not None:
                arguments.append(arg_type(arg_value))
            else:
                arguments.append(None)

        try:
            return self.run(*arguments)
        except Exception as e:
            sys.stderr.write("The script '%s' caused an error while running:\n" % self.__class__.__name__)
            self.__failed = True
            stack_trace = traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_traceback)
            return "".join(stack_trace)
        finally:
            self.cleanup()


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
        predicate = lambda member : inspect.isclass(member) and member.__module__ == module.__name__
        for name, member in inspect.getmembers(module, predicate):
            if ErtScript in inspect.getmro(member):
                result.append(member)

        if len(result) != 1:
            raise UserWarning("Must have (only) one implementation of ErtScript in a module!")

        return result[0]