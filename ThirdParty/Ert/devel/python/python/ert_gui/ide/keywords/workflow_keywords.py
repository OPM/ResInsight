from ert_gui.ide.keywords.definitions import IntegerArgument, KeywordDefinition, ConfigurationLineDefinition, PathArgument, StringArgument, BoolArgument


class WorkflowKeywords(object):
    def __init__(self, ert_keywords):
        super(WorkflowKeywords, self).__init__()
        self.group = "Workflow Jobs"

        ert_keywords.addKeyword(self.addLoadWorkflowJob())
        ert_keywords.addKeyword(self.addWorkflowJobDirectory())
        ert_keywords.addKeyword(self.addLoadWorkflow())
        ert_keywords.addKeyword(self.addInternal())
        ert_keywords.addKeyword(self.addFunction())
        ert_keywords.addKeyword(self.addModule())
        ert_keywords.addKeyword(self.addExecutable())
        ert_keywords.addKeyword(self.addMinArg())
        ert_keywords.addKeyword(self.addMaxArg())
        ert_keywords.addKeyword(self.addArgType())



    def addInternal(self):
        internal = ConfigurationLineDefinition(keyword=KeywordDefinition("INTERNAL"),
                                               arguments=[BoolArgument()],
                                               documentation_link="keywords/internal",
                                               required=False,
                                               group=self.group)
        return internal


    def addFunction(self):
        function = ConfigurationLineDefinition(keyword=KeywordDefinition("FUNCTION"),
                                               arguments=[StringArgument()],
                                               documentation_link="keywords/function",
                                               required=False,
                                               group=self.group)
        return function


    def addModule(self):
        module = ConfigurationLineDefinition(keyword=KeywordDefinition("MODULE"),
                                             arguments=[PathArgument()],
                                             documentation_link="keywords/module",
                                             required=False,
                                             group=self.group)
        return module


    def addExecutable(self):
        executable = ConfigurationLineDefinition(keyword=KeywordDefinition("EXECUTABLE"),
                                                 arguments=[PathArgument()],
                                                 documentation_link="keywords/executable",
                                                 required=False,
                                                 group=self.group)
        return executable




    def addMinArg(self):
        min_arg = ConfigurationLineDefinition(keyword=KeywordDefinition("MIN_ARG"),
                                              arguments=[IntegerArgument()],
                                              documentation_link="keywords/min_arg",
                                              required=False,
                                              group=self.group)
        return min_arg




    def addMaxArg(self):
        max_arg = ConfigurationLineDefinition(keyword=KeywordDefinition("MAX_ARG"),
                                              arguments=[IntegerArgument()],
                                              documentation_link="keywords/max_arg",
                                              required=False,
                                              group=self.group)
        return max_arg



    def addArgType(self):
        arg_type = ConfigurationLineDefinition(keyword=KeywordDefinition("ARG_TYPE"),
                                               arguments=[StringArgument(built_in=True)],
                                               documentation_link="keywords/arg_type",
                                               required=False,
                                               group=self.group)
        return arg_type


    def addLoadWorkflowJob(self):
        load_workflow_job = ConfigurationLineDefinition(keyword=KeywordDefinition("LOAD_WORKFLOW_JOB"),
                                                        arguments=[StringArgument()],
                                                        documentation_link="keywords/load_workflow_job",
                                                        required=False,
                                                        group=self.group)
        return load_workflow_job



    def addWorkflowJobDirectory(self):
        workflow_job_directory = ConfigurationLineDefinition(keyword=KeywordDefinition("WORKFLOW_JOB_DIRECTORY"),
                                                        arguments=[PathArgument()],
                                                        documentation_link="keywords/workflow_job_directory",
                                                        required=False,
                                                        group=self.group)
        return workflow_job_directory


    def addLoadWorkflow(self):
        load_workflow = ConfigurationLineDefinition(keyword=KeywordDefinition("LOAD_WORKFLOW"),
                                                        arguments=[PathArgument(), StringArgument(optional=True)],
                                                        documentation_link="keywords/load_workflow",
                                                        required=False,
                                                        group=self.group)
        return load_workflow