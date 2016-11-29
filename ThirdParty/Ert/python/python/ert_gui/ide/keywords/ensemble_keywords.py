from ert_gui.ide.keywords.definitions import IntegerArgument, KeywordDefinition, ConfigurationLineDefinition, PathArgument, StringArgument


class EnsembleKeywords(object):
    def __init__(self, ert_keywords):
        super(EnsembleKeywords, self).__init__()
        self.group = "Ensemble"

        ert_keywords.addKeyword(self.addNumRealizations())
        ert_keywords.addKeyword(self.addEnsPath())
        ert_keywords.addKeyword(self.addSelectCase())
        ert_keywords.addKeyword(self.addEndDate())
        ert_keywords.addKeyword(self.addHistorySource())
        ert_keywords.addKeyword(self.addRefCase())
        ert_keywords.addKeyword(self.addInclude())
        ert_keywords.addKeyword(self.addObsConfig())
        ert_keywords.addKeyword(self.addResultPath())
        ert_keywords.addKeyword(self.addLicensePath())
        ert_keywords.addKeyword(self.addLocalConfig())
        ert_keywords.addKeyword(self.addRefcaseList())





    def addNumRealizations(self):
        num_realizations = ConfigurationLineDefinition(keyword=KeywordDefinition("NUM_REALIZATIONS"),
                                                       arguments=[IntegerArgument(from_value=1)],
                                                       documentation_link="keywords/num_realizations",
                                                       required=True,
                                                       group=self.group)
        return num_realizations


    def addEndDate(self):
        end_date = ConfigurationLineDefinition(keyword=KeywordDefinition("END_DATE"),
                                                         arguments=[StringArgument()],
                                                         documentation_link="keywords/end_date",
                                                         required=False,
                                                         group=self.group)
        return end_date


    def addEnsPath(self):
        ens_path = ConfigurationLineDefinition(keyword=KeywordDefinition("ENSPATH"),
                                               arguments=[PathArgument()],
                                               documentation_link="keywords/enspath",
                                               required=False,
                                               group=self.group)
        return ens_path



    def addSelectCase(self):
        select_case = ConfigurationLineDefinition(keyword=KeywordDefinition("SELECT_CASE"),
                                                  arguments=[StringArgument()],
                                                  documentation_link="keywords/select_case",
                                                  required=False,
                                                  group=self.group)
        return select_case


    def addHistorySource(self):
        history_source = ConfigurationLineDefinition(keyword=KeywordDefinition("HISTORY_SOURCE"),
                                                  arguments=[StringArgument(built_in=True)],
                                                  documentation_link="keywords/history_source",
                                                  required=False,
                                                  group=self.group)
        return history_source


    def addRefCase(self):
        refcase = ConfigurationLineDefinition(keyword=KeywordDefinition("REFCASE"),
                                                  arguments=[PathArgument()],
                                                  documentation_link="keywords/refcase",
                                                  required=False,
                                                  group=self.group)
        return refcase


    def addRefcaseList(self):
        refcase_list = ConfigurationLineDefinition(keyword=KeywordDefinition("REFCASE_LIST"),
                                                  arguments=[StringArgument(rest_of_line=True,allow_space=True)],
                                                  documentation_link="keywords/refcase_list",
                                                  required=False,
                                                  group=self.group)
        return refcase_list



    def addInclude(self):
        include = ConfigurationLineDefinition(keyword=KeywordDefinition("INCLUDE"),
                                              arguments=[PathArgument()],
                                              documentation_link="keywords/include",
                                              required=False,
                                              group=self.group)
        return include



    def addObsConfig(self):
        obs_config = ConfigurationLineDefinition(keyword=KeywordDefinition("OBS_CONFIG"),
                                                   arguments=[PathArgument()],
                                                   documentation_link="keywords/obs_config",
                                                   required=False,
                                                   group=self.group)
        return obs_config


    def addResultPath(self):
        result_path = ConfigurationLineDefinition(keyword=KeywordDefinition("RESULT_PATH"),
                                                  arguments=[PathArgument()],
                                                  documentation_link="keywords/result_path",
                                                  required=False,
                                                  group=self.group)
        return result_path


    def addLicensePath(self):
        license_path = ConfigurationLineDefinition(keyword=KeywordDefinition("LICENSE_PATH"),
                                                  arguments=[PathArgument()],
                                                  documentation_link="keywords/license_path",
                                                  required=False,
                                                  group=self.group)
        return license_path


    def addLocalConfig(self):
        local_config = ConfigurationLineDefinition(keyword=KeywordDefinition("LOCAL_CONFIG"),
                                                  arguments=[StringArgument(rest_of_line=True,allow_space=True)],
                                                  documentation_link="keywords/local_config",
                                                  required=False,
                                                  group=self.group)
        return local_config

