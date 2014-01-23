from ert_gui.ide.keywords.definitions import IntegerArgument, KeywordDefinition, ConfigurationLineDefinition, PathArgument, StringArgument, BoolArgument


class UnixEnvironmentKeywords(object):
    def __init__(self, ert_keywords):
        super(UnixEnvironmentKeywords, self).__init__()
        self.group = "Unix"

        ert_keywords.addKeyword(self.addSetEnv())
        ert_keywords.addKeyword(self.addUMask())
        ert_keywords.addKeyword(self.addUpdatePath())




    def addSetEnv(self):
        setenv = ConfigurationLineDefinition(keyword=KeywordDefinition("SETENV"),
                                             arguments=[StringArgument(), StringArgument(rest_of_line=True,allow_space=True)],
                                             documentation_link="unix_environment/setenv",
                                             required=False,
                                             group=self.group)
        return setenv


    def addUMask(self):
        umask = ConfigurationLineDefinition(keyword=KeywordDefinition("UMASK"),
                                                  arguments=[IntegerArgument()],
                                                  documentation_link="unix_environment/umask",
                                                  required=False,
                                                  group=self.group)
        return umask


    def addUpdatePath(self):
        update_path = ConfigurationLineDefinition(keyword=KeywordDefinition("UPDATE_PATH"),
                                                  arguments=[StringArgument(built_in=True), PathArgument()],
                                                  documentation_link="unix_environment/update_path",
                                                  required=False,
                                                  group=self.group)
        return update_path
