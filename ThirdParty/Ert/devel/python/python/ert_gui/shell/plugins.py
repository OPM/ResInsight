import inspect
from ert.job_queue import ErtScript, CancelPluginException
from ert_gui.shell import ShellFunction, assertConfigLoaded, autoCompleteList


class Plugins(ShellFunction):
    def __init__(self, shell_context):
        super(Plugins, self).__init__("plugins", shell_context)

        self.addHelpFunction("list", None, "Shows a list of all available plugins.")
        self.addHelpFunction("run", "<plugin_name> [args]", "Run a named plugin with either arguments or default input GUI.")
        self.addHelpFunction("arguments", "<plugin_name>", "Shows a list of expected arguments for a specified plugin.")
        self.addHelpFunction("help", "<plugin_name>", "Shows help for the specified plugin if available.")

    @assertConfigLoaded
    def do_list(self, line):
        plugins = self.getPluginNames()
        if len(plugins) > 0:
            self.columnize(plugins)
        else:
            print("No plugins available.")


    @assertConfigLoaded
    def do_run(self, line):
        arguments = self.splitArguments(line)

        if len(arguments) < 1:
            print("Error: This keyword requires a name of a plugin to run.")
        else:
            plugin_name = arguments[0]
            plugin_job = self.getWorkflowJob(plugin_name)

            if plugin_job is not None:
                try:
                    script = self.getScript(plugin_job)
                    if len(arguments) > 1:
                        arguments = arguments[1:]
                    else:
                        arguments = script.getArguments(None)
                    result = plugin_job.run(self.ert(), arguments)

                    self.shellContext()["debug"].setLastPluginResult(result)

                    print(result)
                except CancelPluginException:
                    print("Plugin cancelled before execution!")
            else:
                self.lastCommandFailed("Unknown plugin: '%s'" % plugin_name)


    @assertConfigLoaded
    def complete_run(self, text, line, begidx, endidx):
        arguments = self.splitArguments(line)

        if len(arguments) > 2 or len(arguments) == 2 and not text:
            return []
        return autoCompleteList(text, self.getPluginNames())


    @assertConfigLoaded
    def do_arguments(self, plugin_name):
        plugin_job = self.getWorkflowJob(plugin_name)

        if plugin_job is not None:
            script, arguments = self.getScriptAndArguments(plugin_job)

            if len(arguments) > 1:
                print("The plugin: '%s' takes the following required <...> and/or optional [...] arguments:\n" % plugin_name)
                arguments_format = " %-25s %-35s %-20s"
                print(arguments_format % ("Arguments", "Default Value", "Type"))

                for argument in arguments:

                    if not argument["optional"]:
                        argument_format = "<%s>"
                    else:
                        argument_format = "[%s]"

                    print(arguments_format % (argument_format % argument["name"], argument["default"], argument["type"].__name__))
            else:
                print("Plugin has no arguments.")
        else:
            self.lastCommandFailed("Unknown plugin: '%s'" % plugin_name)

    @assertConfigLoaded
    def complete_arguments(self, text, line, begidx, endidx):
        arguments = self.splitArguments(line)

        if len(arguments) > 2 or len(arguments) == 2 and not text:
            return []
        return autoCompleteList(text, self.getPluginNames())


    @assertConfigLoaded
    def do_help(self, line):
        arguments = self.splitArguments(line)

        if len(arguments) < 1:
            self.lastCommandFailed("This keyword requires a name of a plugin to run.")
        else:
            plugin_name = arguments[0]
            plugin_job = self.getWorkflowJob(plugin_name)

            if plugin_job is not None:
                script  = self.getScript(plugin_job)

                print(script.__doc__)
            else:
                self.lastCommandFailed("Unknown plugin: '%s'" % plugin_name)


    @assertConfigLoaded
    def complete_help(self, text, line, begidx, endidx):
        arguments = self.splitArguments(line)

        if len(arguments) > 2 or len(arguments) == 2 and not text:
            return []
        return autoCompleteList(text, self.getPluginNames())

    def getPluginNames(self):
        plugin_jobs = self.ert().getWorkflowList().getPluginJobs()
        return [plugin.name() for plugin in plugin_jobs]


    def getWorkflowJob(self, plugin_name):
        """ @rtype: WorkflowJob """
        plugin_name = plugin_name.strip()
        plugin_jobs = self.ert().getWorkflowList().getPluginJobs()
        plugin_job = next((job for job in plugin_jobs if job.name() == plugin_name), None)
        return plugin_job


    def getScript(self, plugin_job):
        script_obj = ErtScript.loadScriptFromFile(plugin_job.getInternalScriptPath())
        script = script_obj(self.ert())
        return script

    def getScriptAndArguments(self, plugin_job):
        script = self.getScript(plugin_job)
        arg_spec = inspect.getargspec(script.run)

        arguments = []

        if len(arg_spec.args) > 1:

            for argument_name, argument_type in zip(arg_spec.args[1:], plugin_job.argumentTypes()):
                arguments.append({
                    "name": argument_name,
                    "type": argument_type,
                    "default": "",
                    "optional": False
                })

            if arg_spec.defaults is not None:
                for index, value in enumerate(reversed(arg_spec.defaults)):
                    arguments[len(arguments) - 1 - index]["default"] = value

            min_arg_count = plugin_job.minimumArgumentCount()

            for index, argument in enumerate(arguments):
                if index >= min_arg_count:
                    argument["optional"] = True

        return script, arguments
