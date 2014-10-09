from ert_gui.ide.keywords.definitions import IntegerArgument, KeywordDefinition, ConfigurationLineDefinition, PathArgument, StringArgument, BoolArgument


class RunKeywords(object):
    def __init__(self, ert_keywords):
        super(RunKeywords, self).__init__()
        self.group = "Run"

        ert_keywords.addKeyword(self.addDeleteRunpath())
        ert_keywords.addKeyword(self.addKeepRunpath())
        ert_keywords.addKeyword(self.addInstallJob())
        ert_keywords.addKeyword(self.addRunpath())
        ert_keywords.addKeyword(self.addRunpathFile())
        ert_keywords.addKeyword(self.addForwardModel())
        ert_keywords.addKeyword(self.addJobScript())
        ert_keywords.addKeyword(self.addRunTemplate())
        ert_keywords.addKeyword(self.addLogLevel())
        ert_keywords.addKeyword(self.addLogFile())
        ert_keywords.addKeyword(self.addMaxSubmit())
        ert_keywords.addKeyword(self.addMaxResample())
        ert_keywords.addKeyword(self.addPreClearRunpath())





    def addInstallJob(self):
        install_job = ConfigurationLineDefinition(keyword=KeywordDefinition("INSTALL_JOB"),
                                                  arguments=[StringArgument(),PathArgument()],
                                                  documentation_link="keywords/install_job",
                                                  required=False,
                                                  group=self.group)
        return install_job



    def addDeleteRunpath(self):
        delete_runpath = ConfigurationLineDefinition(keyword=KeywordDefinition("DELETE_RUNPATH"),
                                                     arguments=[StringArgument()],
                                                     documentation_link="keywords/delete_runpath",
                                                     required=False,
                                                     group=self.group)
        return delete_runpath


    def addKeepRunpath(self):
        keep_runpath = ConfigurationLineDefinition(keyword=KeywordDefinition("KEEP_RUNPATH"),
                                                   arguments=[StringArgument()],
                                                   documentation_link="keywords/keep_runpath",
                                                   required=False,
                                                   group=self.group)
        return keep_runpath





    def addRunpath(self):
        runpath = ConfigurationLineDefinition(keyword=KeywordDefinition("RUNPATH"),
                                                  arguments=[PathArgument(must_exist=False)],
                                                  documentation_link="keywords/runpath",
                                                  required=False,
                                                  group=self.group)
        return runpath



    def addRunpathFile(self):
        runpath_file = ConfigurationLineDefinition(keyword=KeywordDefinition("RUNPATH_FILE"),
                                                  arguments=[PathArgument()],
                                                  documentation_link="keywords/runpath_file",
                                                  required=False,
                                                  group=self.group)
        return runpath_file


    def addForwardModel(self):
        forward_model = ConfigurationLineDefinition(keyword=KeywordDefinition("FORWARD_MODEL"),
                                                    arguments=[StringArgument(rest_of_line=True, allow_space=True)],
                                                    documentation_link="keywords/forward_model",
                                                    required=False,
                                                    group=self.group)
        return forward_model

    def addJobScript(self):
        job_script = ConfigurationLineDefinition(keyword=KeywordDefinition("JOB_SCRIPT"),
                                                 arguments=[PathArgument()],
                                                 documentation_link="keywords/job_script",
                                                 required=False,
                                                 group=self.group)
        return job_script

    def addRunTemplate(self):
        run_template = ConfigurationLineDefinition(keyword=KeywordDefinition("RUN_TEMPLATE"),
                                                 arguments=[PathArgument(),StringArgument()],
                                                 documentation_link="keywords/run_template",
                                                 required=False,
                                                 group=self.group)
        return run_template


    def addLogLevel(self):
        log_level = ConfigurationLineDefinition(keyword=KeywordDefinition("LOG_LEVEL"),
                                                      arguments=[IntegerArgument()],
                                                      documentation_link="keywords/log_level",
                                                      required=False,
                                                      group=self.group)
        return log_level


    def addLogFile(self):
        log_file = ConfigurationLineDefinition(keyword=KeywordDefinition("LOG_FILE"),
                                                      arguments=[PathArgument()],
                                                      documentation_link="keywords/log_file",
                                                      required=False,
                                                      group=self.group)
        return log_file



    def addMaxSubmit(self):
        max_submit = ConfigurationLineDefinition(keyword = KeywordDefinition("MAX_SUBMIT"),
                                                      arguments=[IntegerArgument()],
                                                      documentation_link="keywords/max_submit",
                                                      group=self.group)
        return max_submit


    def addMaxResample(self):
        max_resample = ConfigurationLineDefinition(keyword=KeywordDefinition("MAX_RESAMPLE"),
                                                  arguments=[IntegerArgument()],
                                                  documentation_link="keywords/max_resample",
                                                  required=False,
                                                  group=self.group)
        return max_resample


    def addPreClearRunpath(self):
        pre_clear_runpath = ConfigurationLineDefinition(keyword=KeywordDefinition("PRE_CLEAR_RUNPATH"),
                                                  arguments=[BoolArgument()],
                                                  documentation_link="keywords/pre_clear_runpath",
                                                  required=False,
                                                  group=self.group)
        return pre_clear_runpath