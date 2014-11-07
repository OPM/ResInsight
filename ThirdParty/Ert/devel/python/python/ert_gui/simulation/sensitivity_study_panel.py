'''
Created on 8. juli 2014

@author: perroe
'''

from PyQt4.QtGui import QFormLayout, QToolButton
from ert_gui.ide.keywords.definitions import RangeStringArgument, \
    ProperNameFormatStringArgument
from ert_gui.models.connectors import EnsembleSizeModel
from ert_gui.models.connectors.init import CaseSelectorModel
from ert_gui.models.connectors.run import SensitivityStudy, \
    ActiveRealizationsModel, RunPathModel, SensitivityTargetCaseFormatModel
from ert_gui.simulation import SensitivityStudyParametersPanel
from ert_gui.simulation.simulation_config_panel import SimulationConfigPanel
from ert_gui.widgets import util
from ert_gui.widgets.active_label import ActiveLabel
from ert_gui.widgets.closable_dialog import ClosableDialog
from ert_gui.widgets.combo_choice import ComboChoice
from ert_gui.widgets.string_box import StringBox


class SensitivityStudyPanel(SimulationConfigPanel):
    '''
    Panel for setting parameters for sensitivity study.
    '''


    def __init__(self):
        '''
        Fills in the input panel for sensitivity study parameters.
        '''

        SimulationConfigPanel.__init__(self, SensitivityStudy())

        layout = QFormLayout()

        case_model = CaseSelectorModel()
        case_selector = ComboChoice(case_model, "Current case", "init/current_case_selection")
        layout.addRow(case_selector.getLabel(), case_selector)

        runpath_model = RunPathModel()
        runpath_label = ActiveLabel(runpath_model, "Runpath", "config/simulation/runpath")
        layout.addRow(runpath_label.getLabel(), runpath_label)

        number_of_realizations_model = EnsembleSizeModel()
        number_of_realizations_label = ActiveLabel(number_of_realizations_model, "Number of realizations", "config/ensemble/num_realizations")
        layout.addRow(number_of_realizations_label.getLabel(), number_of_realizations_label)

        sensitivity_target_case_format_model = SensitivityTargetCaseFormatModel()
        self.iterated_target_case_format_field = StringBox(sensitivity_target_case_format_model, "Target case format",
                                                           "config/simulation/sensitivity_target_case_format")
        self.iterated_target_case_format_field.setValidator(ProperNameFormatStringArgument())
        layout.addRow(self.iterated_target_case_format_field.getLabel(), self.iterated_target_case_format_field)

        self.parameters_popup_button = QToolButton()
        self.parameters_popup_button.setIcon(util.resourceIcon("ide/small/cog_edit.png"))
        self.parameters_popup_button.clicked.connect(self.showParametersPopup)
        self.parameters_popup_button.setMaximumSize(20, 20)

        layout.addRow("Parameters:", self.parameters_popup_button)

        active_realizations_model = ActiveRealizationsModel()
        self.active_realizations_field = StringBox(active_realizations_model, "Active realizations", "config/simulation/active_realizations")
        self.active_realizations_field.setValidator(RangeStringArgument(number_of_realizations_model.getValue()))
        layout.addRow(self.active_realizations_field.getLabel(), self.active_realizations_field)

        self.active_realizations_field.validationChanged.connect(self.simulationConfigurationChanged)

        self.setLayout(layout)

    def isConfigurationValid(self):
        '''
        Check if the given input configuration is valid, and that all needed
        data is given.
        '''

        return self.active_realizations_field.isValid()

    def showParametersPopup(self):
        parameter_panel = SensitivityStudyParametersPanel()
        dialog = ClosableDialog("Sensitivity Study Parameters", parameter_panel, self.parent())

        dialog.exec_()


    def toggleAdvancedOptions(self, show_advanced):
        self.active_realizations_field.setVisible(show_advanced)
        self.layout().labelForField(self.active_realizations_field).setVisible(show_advanced)
