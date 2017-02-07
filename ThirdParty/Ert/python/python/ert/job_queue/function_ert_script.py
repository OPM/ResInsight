import ert as ert_module
from cwrap import Prototype

from ert.job_queue import ErtScript
from ert.util.stringlist import StringList


class _NonePrototype(Prototype):
    lib = ert_module.load(None)

    def __init__(self, prototype, bind=True):
        super(_NonePrototype, self).__init__(_NonePrototype.lib, prototype, bind=bind)


class FunctionErtScript(ErtScript):

    def __init__(self, ert, function_name, argument_types, argument_count):
        super(FunctionErtScript, self).__init__(ert)

        parsed_argument_types = []

        if ert is not None:
            self.__function = _NonePrototype("void* %s(void*, stringlist)" % function_name)

        else:
            for arg in argument_types:
                if arg is bool:
                    parsed_argument_types.append("bool")
                elif arg is str:
                    parsed_argument_types.append("char*")
                elif arg is int:
                    parsed_argument_types.append("int")
                elif arg is float:
                    parsed_argument_types.append("float")
                else:
                    raise TypeError("Unknown type: %s" % arg)

            self.__function = _NonePrototype("void* %s(%s)" % (function_name, ", ".join(parsed_argument_types[:argument_count])))


    def run(self, *args):
        ert = self.ert()
        if ert is None:
            # This is usually used for testing purposes without an ert instance
            return self.__function(*args)
        else:
            str_args = StringList()
            for arg in args:
                str_args.append(arg)

            if hasattr(ert, "from_param"):
                pointer = ert.from_param(ert)
            else:
                pointer = ert # ...

            return self.__function(pointer, str_args)

    def cancel(self):
        # job is not cancellable and will just ignore the call
        pass




