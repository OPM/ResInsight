from PyQt4.QtGui import QFormLayout
from ert_gui.ide.keywords.definitions import RangeStringArgument, ProperNameArgument
from ert_gui.models.connectors import EnsembleSizeModel
from ert_gui.models.connectors.init import CaseSelectorModel
from ert_gui.models.connectors.run import ActiveRealizationsModel, EnsembleSmoother, TargetCaseModel, AnalysisModuleModel, RunPathModel
from ert_gui.simulation import SimulationConfigPanel
from ert_gui.widgets.active_label import ActiveLabel
from ert_gui.widgets.combo_choice import ComboChoice
from ert_gui.widgets.string_box import StringBox


class EnsembleSmootherPanel(SimulationConfigPanel):
    def __init__(self):
        SimulationConfigPanel.__init__(self, EnsembleSmoother())

        layout = QFormLayout()

        case_model = CaseSelectorModel()
        case_selector = ComboChoice(case_model, "Current case", "init/current_case_selection")
        layout.addRow(case_selector.getLabel(), case_selector)

        run_path_model = RunPathModel()
        run_path_label = ActiveLabel(run_path_model, "Runpath", "config/simulation/runpath")
        layout.addRow(run_path_label.getLabel(), run_path_label)

        # re_run_path_model = RerunPathModel()
        # re_run_path_label = ActiveLabel(re_run_path_model, "Rerunpath", "config/simulation/rerunpath")
        # layout.addRow(re_run_path_label.getLabel(), re_run_path_label)


        number_of_realizations_model = EnsembleSizeModel()
        number_of_realizations_label = ActiveLabel(number_of_realizations_model, "Number of realizations", "config/ensemble/num_realizations")
        layout.addRow(number_of_realizations_label.getLabel(), number_of_realizations_label)

        analysis_module_model = AnalysisModuleModel()
        analysis_module_choice = ComboChoice(analysis_module_model, "Analysis Module", "config/analysis/analysis_module")
        layout.addRow(analysis_module_choice.getLabel(), analysis_module_choice)


        target_case_model = TargetCaseModel()
        self.target_case_field = StringBox(target_case_model, "Target case", "config/simulation/target_case")
        self.target_case_field.setValidator(ProperNameArgument())
        layout.addRow(self.target_case_field.getLabel(), self.target_case_field)

        active_realizations_model = ActiveRealizationsModel()
        self.active_realizations_field = StringBox(active_realizations_model, "Active realizations", "config/simulation/active_realizations")
        self.active_realizations_field.setValidator(RangeStringArgument())
        layout.addRow(self.active_realizations_field.getLabel(), self.active_realizations_field)


        self.target_case_field.validationChanged.connect(self.simulationConfigurationChanged)
        self.active_realizations_field.validationChanged.connect(self.simulationConfigurationChanged)

        self.setLayout(layout)

    def isConfigurationValid(self):
        return self.target_case_field.isValid() and self.active_realizations_field.isValid()




