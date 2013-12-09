from ert_gui.ide.keywords.definitions import IntegerArgument, KeywordDefinition, ConfigurationLineDefinition, PathArgument, StringArgument, FloatArgument, BoolArgument
from ert_gui.ide.keywords.definitions.proper_name_format_argument import ProperNameFormatArgument


class AnalysisModuleKeywords(object):
    def __init__(self, ert_keywords):
        super(AnalysisModuleKeywords, self).__init__()
        self.group = "Analysis Module"

        ert_keywords.addKeyword(self.addAnalysisLoad())
        ert_keywords.addKeyword(self.addAnalysisSelect())
        ert_keywords.addKeyword(self.addAnalysisCopy())
        ert_keywords.addKeyword(self.addAnalysisSetVar())
        ert_keywords.addKeyword(self.addIterRunpath())
        ert_keywords.addKeyword(self.addIterCase())
        ert_keywords.addKeyword(self.addIterCount())
        ert_keywords.addKeyword(self.addStdCutoff())
        ert_keywords.addKeyword(self.addSingleNodeUpdate())



    def addAnalysisLoad(self):
        analysis_load = ConfigurationLineDefinition(keyword=KeywordDefinition("ANALYSIS_LOAD"),
                                                    arguments=[StringArgument(),StringArgument()],
                                                    documentation_link="analysis_module/analysis_load",
                                                    required=False,
                                                    group=self.group)
        return analysis_load



    def addAnalysisSelect(self):
        analysis_select = ConfigurationLineDefinition(keyword=KeywordDefinition("ANALYSIS_SELECT"),
                                                      arguments=[StringArgument()],
                                                      documentation_link="analysis_module/analysis_select",
                                                      required=False,
                                                      group=self.group)
        return analysis_select


    def addAnalysisSetVar(self):
        analysis_set_var = ConfigurationLineDefinition(keyword=KeywordDefinition("ANALYSIS_SET_VAR"),
                                                       arguments=[StringArgument(),
                                                                  StringArgument(),
                                                                  StringArgument(rest_of_line=True,allow_space=True)],
                                                       documentation_link="analysis_module/analysis_set_var",
                                                       required=False,
                                                       group=self.group)
        return analysis_set_var



    def addAnalysisCopy(self):
        analysis_copy = ConfigurationLineDefinition(keyword=KeywordDefinition("ANALYSIS_COPY"),
                                                    arguments=[StringArgument(), StringArgument()],
                                                    documentation_link="analysis_module/analysis_copy",
                                                    required=False,
                                                    group=self.group)
        return analysis_copy



    def addIterRunpath(self):
        iter_runpath = ConfigurationLineDefinition(keyword=KeywordDefinition("ITER_RUNPATH"),
                                                   arguments=[PathArgument()],
                                                   documentation_link="analysis_module/iter_runpath",
                                                   required=False,
                                                   group=self.group)
        return iter_runpath


    def addIterCase(self):
        iter_case = ConfigurationLineDefinition(keyword=KeywordDefinition("ITER_CASE"),
                                                   arguments=[ProperNameFormatArgument()],
                                                   documentation_link="analysis_module/iter_case",
                                                   required=False,
                                                   group=self.group)
        return iter_case


    def addIterCount(self):
        iter_count = ConfigurationLineDefinition(keyword=KeywordDefinition("ITER_COUNT"),
                                                 arguments=[IntegerArgument()],
                                                 documentation_link="analysis_module/iter_count",
                                                 required=False,
                                                 group=self.group)
        return iter_count


    def addStdCutoff(self):
        std_cutoff = ConfigurationLineDefinition(keyword=KeywordDefinition("STD_CUTOFF"),
                                                 arguments=[FloatArgument()],
                                                 documentation_link="analysis_module/std_cutoff",
                                                 required=False,
                                                 group=self.group)
        return std_cutoff


    def addSingleNodeUpdate(self):
        single_node_update = ConfigurationLineDefinition(keyword=KeywordDefinition("SINGLE_NODE_UPDATE"),
                                                 arguments=[BoolArgument()],
                                                 documentation_link="analysis_module/single_node_update",
                                                 required=False,
                                                 group=self.group)
        return single_node_update