from ert_gui.ide.keywords.definitions import IntegerArgument, KeywordDefinition, ConfigurationLineDefinition, PathArgument, StringArgument, BoolArgument


class ReportKeywords(object):
    def __init__(self, ert_keywords):
        super(ReportKeywords, self).__init__()
        self.group = "Report"

        ert_keywords.addKeyword(self.addReportContext())
        ert_keywords.addKeyword(self.addReportSearchPath())
        ert_keywords.addKeyword(self.addReportList())
        ert_keywords.addKeyword(self.addReportPath())
        ert_keywords.addKeyword(self.addReportWellList())
        ert_keywords.addKeyword(self.addReportGroupList())
        ert_keywords.addKeyword(self.addReportTimeout())
        ert_keywords.addKeyword(self.addReportLarge())




    def addReportContext(self):
        report_context = ConfigurationLineDefinition(keyword=KeywordDefinition("REPORT_CONTEXT"),
                                                     arguments=[StringArgument(), StringArgument(rest_of_line=True,allow_space=True)],
                                                     documentation_link="report/report_context",
                                                     required=False,
                                                     group=self.group)
        return report_context


    def addReportList(self):
        report_list = ConfigurationLineDefinition(keyword=KeywordDefinition("REPORT_LIST"),
                                                  arguments=[StringArgument(rest_of_line=True,allow_space=True)],
                                                  documentation_link="report/report_list",
                                                  required=False,
                                                  group=self.group)
        return report_list


    def addReportPath(self):
        report_path = ConfigurationLineDefinition(keyword=KeywordDefinition("REPORT_PATH"),
                                                  arguments=[PathArgument()],
                                                  documentation_link="report/report_path",
                                                  required=False,
                                                  group=self.group)
        return report_path




    def addReportSearchPath(self):
        report_search_path = ConfigurationLineDefinition(keyword=KeywordDefinition("REPORT_SEARCH_PATH"),
                                                     arguments=[StringArgument(rest_of_line=True,allow_space=True)],
                                                     documentation_link="report/report_search_path",
                                                     required=False,
                                                     group=self.group)
        return report_search_path


    def addReportWellList(self):
        report_well_list = ConfigurationLineDefinition(keyword=KeywordDefinition("REPORT_WELL_LIST"),
                                                     arguments=[StringArgument(),StringArgument(rest_of_line=True,allow_space=True)],
                                                     documentation_link="report/report_well_list",
                                                     required=False,
                                                     group=self.group)
        return report_well_list


    def addReportGroupList(self):
        report_group_list = ConfigurationLineDefinition(keyword=KeywordDefinition("REPORT_GROUP_LIST"),
                                                     arguments=[StringArgument(),StringArgument(rest_of_line=True,allow_space=True)],
                                                     documentation_link="report/report_group_list",
                                                     required=False,
                                                     group=self.group)
        return report_group_list

    def addReportTimeout(self):
        report_timeout = ConfigurationLineDefinition(keyword=KeywordDefinition("REPORT_TIMEOUT"),
                                                     arguments=[IntegerArgument()],
                                                     documentation_link="report/report_timeout",
                                                     required=False,
                                                     group=self.group)
        return report_timeout


    def addReportLarge(self):
        report_large = ConfigurationLineDefinition(keyword=KeywordDefinition("REPORT_LARGE"),
                                                     arguments=[BoolArgument()],
                                                     documentation_link="report/report_large",
                                                     required=False,
                                                     group=self.group)
        return report_large