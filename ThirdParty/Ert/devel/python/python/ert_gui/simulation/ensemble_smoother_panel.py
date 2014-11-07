from PyQt4.QtCore import Qt, QMargins
from PyQt4.QtGui import QFormLayout, QToolButton, QHBoxLayout
from ert_gui.ide.keywords.definitions import RangeStringArgument, ProperNameArgument
from ert_gui.models.connectors import EnsembleSizeModel
from ert_gui.models.connectors.init import CaseSelectorModel
from ert_gui.models.connectors.run import ActiveRealizationsModel, EnsembleSmoother, TargetCaseModel, AnalysisModuleModel, RunPathModel
from ert_gui.simulation import SimulationConfigPanel, AnalysisModuleVariablesPanel
from ert_gui.widgets import util
from ert_gui.widgets.active_label import ActiveLabel
from ert_gui.widgets.closable_dialog import ClosableDialog
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

        number_of_realizations_model = EnsembleSizeModel()
        number_of_realizations_label = ActiveLabel(number_of_realizations_model, "Number of realizations", "config/ensemble/num_realizations")
        layout.addRow(number_of_realizations_label.getLabel(), number_of_realizations_label)

        target_case_model = TargetCaseModel()
        self.target_case_field = StringBox(target_case_model, "Target case", "config/simulation/target_case")
        self.target_case_field.setValidator(ProperNameArgument())
        layout.addRow(self.target_case_field.getLabel(), self.target_case_field)

        analysis_module_model = AnalysisModuleModel()
        self.analysis_module_choice = ComboChoice(analysis_module_model, "Analysis Module", "config/analysis/analysis_module")

        self.variables_popup_button = QToolButton()
        self.variables_popup_button.setIcon(util.resourceIcon("ide/small/cog_edit.png"))
        self.variables_popup_button.clicked.connect(self.showVariablesPopup)
        self.variables_popup_button.setMaximumSize(20, 20)

        self.variables_layout = QHBoxLayout()
        self.variables_layout.addWidget(self.analysis_module_choice, 0, Qt.AlignLeft)
        self.variables_layout.addWidget(self.variables_popup_button, 0, Qt.AlignLeft)
        self.variables_layout.setContentsMargins(QMargins(0,0,0,0))
        self.variables_layout.addStretch()

        layout.addRow(self.analysis_module_choice.getLabel(), self.variables_layout)

        active_realizations_model = ActiveRealizationsModel()
        self.active_realizations_field = StringBox(active_realizations_model, "Active realizations", "config/simulation/active_realizations")
        self.active_realizations_field.setValidator(RangeStringArgument())
        layout.addRow(self.active_realizations_field.getLabel(), self.active_realizations_field)

        self.target_case_field.validationChanged.connect(self.simulationConfigurationChanged)
        self.active_realizations_field.validationChanged.connect(self.simulationConfigurationChanged)

        self.setLayout(layout)

    def isConfigurationValid(self):
        return self.target_case_field.isValid() and self.active_realizations_field.isValid()

    def toggleAdvancedOptions(self, show_advanced):
        self.active_realizations_field.setVisible(show_advanced)
        self.layout().labelForField(self.active_realizations_field).setVisible(show_advanced)

        self.analysis_module_choice.setVisible(show_advanced)
        self.layout().labelForField(self.variables_layout).setVisible(show_advanced)
        self.variables_popup_button.setVisible(show_advanced)

    def showVariablesPopup(self):
        analysis_module_name = AnalysisModuleModel().getCurrentChoice()
        if analysis_module_name is not None:
            variable_dialog = AnalysisModuleVariablesPanel(analysis_module_name)
            dialog = ClosableDialog("Edit variables", variable_dialog, self.parent())

            dialog.exec_()


