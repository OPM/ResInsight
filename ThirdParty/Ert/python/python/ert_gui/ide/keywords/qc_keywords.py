from ert_gui.ide.keywords.definitions import IntegerArgument, KeywordDefinition, ConfigurationLineDefinition, PathArgument, StringArgument


class QCKeywords(object):
    def __init__(self, ert_keywords):
        super(QCKeywords, self).__init__()
        self.group = "Quality Check"

        ert_keywords.addKeyword(self.addQCWorkflow())
        ert_keywords.addKeyword(self.addQCPath())




    def addQCWorkflow(self):
        qc_workflow = ConfigurationLineDefinition(keyword=KeywordDefinition("QC_WORKFLOW"),
                                                    arguments=[StringArgument()],
                                                    documentation_link="keywords/qc_workflow",
                                                    required=False,
                                                    group=self.group)
        return qc_workflow



    def addQCPath(self):
        qc_path = ConfigurationLineDefinition(keyword=KeywordDefinition("QC_PATH"),
                                                    arguments=[PathArgument()],
                                                    documentation_link="keywords/qc_path",
                                                    required=False,
                                                    group=self.group)
        return qc_path