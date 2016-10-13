from ert import Version
from ert_gui.shell import assertConfigLoaded, ErtShellCollection


class Debug(ErtShellCollection):
    def __init__(self, parent):
        super(Debug, self).__init__("debug", parent)

        self.addShellFunction(name="site_config", function=Debug.siteConfig, help_message="Show the path to the current site_config")
        self.addShellFunction(name="version", function=Debug.version, help_message="Show the internalized ert version number")
        self.addShellFunction(name="timestamp", function=Debug.timestamp, help_message="Show the build timestamp")
        self.addShellFunction(name="git_commit", function=Debug.gitCommit, help_message="Show the git commit")
        self.addShellFunction(name="info", function=Debug.info, help_message="Shows site_config, version, timestamp and Git Commit")
        self.addShellFunction(name="last_plugin_result", function=Debug.lastPluginResult, help_message="Shows the last plugin result.")
        self.addShellFunction(name="eval", function=Debug.eval, help_arguments="<Python expression>", help_message="Evaluate a Python expression. The last plugin result is defined as: x")

        self.shellContext()["debug"] = self
        self.__last_plugin_result = None
        self.__local_variables = {}

    def setLastPluginResult(self, result):
        self.__last_plugin_result = result

    @assertConfigLoaded
    def siteConfig(self, line):
        print("Site Config: %s" % self.ert().siteConfig().getLocation())

    def version(self, line):
        print("Version: %s" % Version.getVersion())

    def timestamp(self, line):
        print("Timestamp: %s" % Version.getBuildTime())

    def gitCommit(self, line):
        print("Git Commit: %s" % Version.getGitCommit(True))

    @assertConfigLoaded
    def info(self, line):
        print("Site Config: %s" % self.ert().siteConfig().getLocation())
        print("Version:     %s" % Version.getVersion())
        print("Timestamp:   %s" % Version.getBuildTime())
        print("Git Commit:  %s" % Version.getGitCommit(True))

    def lastPluginResult(self, line):
        print("Last plugin result: %s" % self.__last_plugin_result)

    def eval(self, line):
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

