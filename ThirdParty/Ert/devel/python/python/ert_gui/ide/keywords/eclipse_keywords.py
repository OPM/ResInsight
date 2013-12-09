from ert_gui.ide.keywords.definitions import IntegerArgument, KeywordDefinition, ConfigurationLineDefinition, StringArgument, BoolArgument
from ert_gui.ide.keywords.definitions.path_argument import PathArgument


class EclipseKeywords(object):
    def __init__(self, ert_keywords):
        super(EclipseKeywords, self).__init__()
        self.group = "Eclipse"


        ert_keywords.addKeyword(self.addDataFile())
        ert_keywords.addKeyword(self.addEclBase())
        ert_keywords.addKeyword(self.addJobName())
        ert_keywords.addKeyword(self.addGrid())
        ert_keywords.addKeyword(self.addInitSection())
        ert_keywords.addKeyword(self.addScheduleFile())
        ert_keywords.addKeyword(self.addDataKw())
        ert_keywords.addKeyword(self.addEquilInitFile())
        ert_keywords.addKeyword(self.addIgnoreSchedule())



    def addDataFile(self):
        data_file = ConfigurationLineDefinition(keyword=KeywordDefinition("DATA_FILE"),
                                                arguments=[PathArgument()],
                                                documentation_link="eclipse/data_file",
                                                required=True,
                                                group=self.group)
        return data_file


    def addEquilInitFile(self):
        equil_init_file = ConfigurationLineDefinition(keyword=KeywordDefinition("EQUIL_INIT_FILE"),
                                                      arguments=[PathArgument()],
                                                      documentation_link="eclipse/equil_init_file",
                                                      group=self.group)
        return equil_init_file



    def addEclBase(self):
        ecl_base = ConfigurationLineDefinition(keyword=KeywordDefinition("ECLBASE"),
                                               arguments=[StringArgument()],
                                               documentation_link="eclipse/ecl_base",
                                               required=True,
                                               group=self.group)
        return ecl_base


    def addJobName(self):
        job_name = ConfigurationLineDefinition(keyword=KeywordDefinition("JOBNAME"),
                                               arguments=[StringArgument()],
                                               documentation_link="eclipse/job_name",
                                               required=True,
                                               group=self.group)
        return job_name


    def addGrid(self):
        grid = ConfigurationLineDefinition(keyword=KeywordDefinition("GRID"),
                                           arguments=[PathArgument()],
                                           documentation_link="eclipse/grid",
                                           required=True,
                                           group=self.group)
        return grid

    def addInitSection(self):
        init_section = ConfigurationLineDefinition(keyword=KeywordDefinition("INIT_SECTION"),
                                                   arguments=[PathArgument()],
                                                   documentation_link="eclipse/init_section",
                                                   required=True,
                                                   group=self.group)
        return init_section


    def addScheduleFile(self):
        schedule_file = ConfigurationLineDefinition(keyword=KeywordDefinition("SCHEDULE_FILE"),
                                                    arguments=[PathArgument()],
                                                    documentation_link="ensemble/schedule_file",
                                                    required=True,
                                                    group=self.group)
        return schedule_file


    def addIgnoreSchedule(self):
        ignore_schedule = ConfigurationLineDefinition(keyword=KeywordDefinition("IGNORE_SCHEDULE"),
                                                    arguments=[BoolArgument()],
                                                    documentation_link="ensemble/ignore_schedule",
                                                    required=False,
                                                    group=self.group)
        return ignore_schedule


    def addDataKw(self):
        data_kw = ConfigurationLineDefinition(keyword=KeywordDefinition("DATA_KW"),
                                                    arguments=[StringArgument(),
                                                               StringArgument(rest_of_line=True, allow_space=True)],
                                                    documentation_link="ensemble/data_kw",
                                                    required=False,
                                                    group=self.group)
        return data_kw