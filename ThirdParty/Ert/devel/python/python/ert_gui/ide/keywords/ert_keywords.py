from ert_gui.ide.keywords.advanced_keywords import AdvancedKeywords
from ert_gui.ide.keywords.analysis_module_keywords import AnalysisModuleKeywords
from ert_gui.ide.keywords.definitions import ConfigurationLineDefinition
from ert_gui.ide.keywords.eclipse_keywords import EclipseKeywords
from ert_gui.ide.keywords.enkf_control_keywords import EnkfControlKeywords
from ert_gui.ide.keywords.ensemble_keywords import EnsembleKeywords
from ert_gui.ide.keywords.parametrization_keywords import ParametrizationKeywords
from ert_gui.ide.keywords.plot_keywords import PlotKeywords
from ert_gui.ide.keywords.qc_keywords import QCKeywords
from ert_gui.ide.keywords.queue_system_keywords import QueueSystemKeywords
from ert_gui.ide.keywords.run_keywords import RunKeywords
from ert_gui.ide.keywords.simulation_control_keywords import SimulationControlKeywords
from ert_gui.ide.keywords.unix_environment_keywords import UnixEnvironmentKeywords
from ert_gui.ide.keywords.workflow_keywords import WorkflowKeywords


class ErtKeywords(object):
    def __init__(self):
        super(ErtKeywords, self).__init__()

        self.keywords = {}
        self.groups = {}

        EnsembleKeywords(self)
        RunKeywords(self)
        EclipseKeywords(self)
        QueueSystemKeywords(self)
        SimulationControlKeywords(self)
        ParametrizationKeywords(self)
        EnkfControlKeywords(self)
        AnalysisModuleKeywords(self)
        PlotKeywords(self)
        WorkflowKeywords(self)
        AdvancedKeywords(self)
        QCKeywords(self)
        UnixEnvironmentKeywords(self)


        # group_names = sorted(self.groups.keys())
        #
        # for group in group_names:
        #     print(group)
        #     keywords = self.groups[group]
        #     for keyword in keywords:
        #         print("  %s" % keyword.keywordDefinition().name())





    def addKeyword(self, keyword):
        assert isinstance(keyword, ConfigurationLineDefinition)

        name = keyword.keywordDefinition().name()
        if name in self.keywords:
            raise ValueError("Keyword %s already in Ert keyword list!" % name)

        self.keywords[name] = keyword

        group = keyword.group()

        if not group in self.groups:
            self.groups[group] = []

        self.groups[group].append(keyword)

    def __contains__(self, item):
        return item in self.keywords

    def __getitem__(self, item):
        """ @rtype: ConfigurationLineDefinition """
        return self.keywords[item]

