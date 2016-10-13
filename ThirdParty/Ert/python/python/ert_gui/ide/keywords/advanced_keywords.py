from ert_gui.ide.keywords.definitions import IntegerArgument, KeywordDefinition, ConfigurationLineDefinition, PathArgument, StringArgument
from ert_gui.ide.keywords.definitions.proper_name_argument import ProperNameArgument


class AdvancedKeywords(object):
    def __init__(self, ert_keywords):
        super(AdvancedKeywords, self).__init__()
        self.group = "Advanced"

        ert_keywords.addKeyword(self.addDefine())
        ert_keywords.addKeyword(self.addSchedulePredictionFile())
        ert_keywords.addKeyword(self.addFixedLengthScheduleKw())
        ert_keywords.addKeyword(self.addStaticKw())



    def addFixedLengthScheduleKw(self):
        fixed_length_sched_kw = ConfigurationLineDefinition(keyword=KeywordDefinition("ADD_FIXED_LENGTH_SCHEDULE_KW"),
                                             arguments=[StringArgument(),
                                                        StringArgument()],
                                             documentation_link="keywords/add_fixed_length_schedule_kw",
                                             required=False,
                                             group=self.group)
        return fixed_length_sched_kw


    def addStaticKw(self):
        add_static_kw = ConfigurationLineDefinition(keyword=KeywordDefinition("ADD_STATIC_KW"),
                                             arguments=[StringArgument(),
                                                        StringArgument()],
                                             documentation_link="keywords/add_static_kw",
                                             required=False,
                                             group=self.group)
        return add_static_kw



    def addDefine(self):
        define = ConfigurationLineDefinition(keyword=KeywordDefinition("DEFINE"),
                                             arguments=[ProperNameArgument(),
                                                        StringArgument(rest_of_line=True,allow_space=True)],
                                             documentation_link="keywords/define",
                                             required=False,
                                             group=self.group)
        return define



    def addSchedulePredictionFile(self):
        schedule_prediction_file = ConfigurationLineDefinition(keyword=KeywordDefinition("SCHEDULE_PREDICTION_FILE"),
                                                      arguments=[PathArgument()],
                                                      documentation_link="keywords/schedule_prediction_file",
                                                      required=False,
                                                      group=self.group)
        return schedule_prediction_file

