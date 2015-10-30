from ert import Version
from ert_gui.shell import ShellFunction, assertConfigLoaded


class Debug(ShellFunction):
    def __init__(self, shell_context):
        super(Debug, self).__init__("debug", shell_context)
        self.addHelpFunction("site_config", None, "Show the path to the current site_config")
        self.addHelpFunction("version", None, "Show the internalized ert version number")
        self.addHelpFunction("timestamp", None, "Show the build timestamp")
        self.addHelpFunction("git_commit", None, "Show the git commit")
        self.addHelpFunction("info", None, "Shows site_config, version, timestamp and Git Commit")
        self.addHelpFunction("last_plugin_result", None, "Shows the last plugin result.")
        self.addHelpFunction("eval", "<Python expression>", "Evaluate a Python expression. "
                                                            "The last plugin result is defined as: x")

        shell_context["debug"] = self
        self.__last_plugin_result = None
        self.__local_variables = {}


    def setLastPluginResult(self, result):
        self.__last_plugin_result = result

    @assertConfigLoaded
    def do_site_config(self, line):
        print("Site Config: %s" % self.ert().siteConfig().getLocation())

    def do_version(self, line):
        print("Version: %s" % Version.getVersion())

    def do_timestamp(self, line):
        print("Timestamp: %s" % Version.getBuildTime())

    def do_git_commit(self, line):
        print("Git Commit: %s" % Version.getGitCommit(True))

    @assertConfigLoaded
    def do_info(self, line):
        print("Site Config: %s" % self.ert().siteConfig().getLocation())
        print("Version:     %s" % Version.getVersion())
        print("Timestamp:   %s" % Version.getBuildTime())
        print("Git Commit:  %s" % Version.getGitCommit(True))

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

