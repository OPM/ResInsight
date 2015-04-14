from ert_gui.shell import ShellFunction, assertConfigLoaded


class Debug(ShellFunction):
    def __init__(self, shell_context):
        super(Debug, self).__init__("debug", shell_context)
        self.addHelpFunction("last_plugin_result", None, "Shows the last plugin result.")
        self.addHelpFunction("eval", "<Python expression>", "Evaluate a Python expression. "
                                                            "The last plugin result is defined as: x")

        shell_context["debug"] = self
        self.__last_plugin_result = None
        self.__local_variables = {}


    def setLastPluginResult(self, result):
        self.__last_plugin_result = result

    def do_last_plugin_result(self, line):
        print("Last plugin result: %s" % self.__last_plugin_result)

    def do_eval(self, line):
        line = line.strip()

        if len(line) > 0:
            self.__local_variables["x"] = self.__last_plugin_result
            try:
                exec(line, self.__local_variables)
            except Exception as e:
                print("Error: The expression caused an exception!")
                print(e)
        else:
            print("Error: A python expression is required!")

