from ert_gui.ide.keywords.definitions import IntegerArgument, BoolArgument, StringArgument, ConfigurationLineDefinition, KeywordDefinition , PercentArgument


class SimulationControlKeywords(object):
    def __init__(self, ert_keywords):
        super(SimulationControlKeywords, self).__init__()
        self.group = "Simulation Control"



        ert_keywords.addKeyword(self.addMaxRuntime())
        ert_keywords.addKeyword(self.addMinRealizations())
        ert_keywords.addKeyword(self.addStopLongRunning())


    def addMaxRuntime(self):
        max_runtime = ConfigurationLineDefinition(keyword=KeywordDefinition("MAX_RUNTIME"),
                                                   arguments=[IntegerArgument(from_value=0)],
                                                   documentation_link="keywords/max_runtime",
                                                   required=False,
                                                   group=self.group)
        return max_runtime




    def addMinRealizations(self):
        min_realizations = ConfigurationLineDefinition(keyword=KeywordDefinition("MIN_REALIZATIONS"),
                                                       arguments=[IntegerArgument(from_value = 0) , 
                                                                  PercentArgument(from_value = 0 , to_value = 100)],
                                                       documentation_link="keywords/min_realizations",
                                                       required=False,
                                                       group=self.group)
        return min_realizations


    def addStopLongRunning(self):
        stop_long_running = ConfigurationLineDefinition(keyword=KeywordDefinition("STOP_LONG_RUNNING"),
                                                        arguments=[BoolArgument()],
                                                        documentation_link="keywords/stop_long_running",
                                                        required=False,
                                                        group=self.group)
        return stop_long_running
