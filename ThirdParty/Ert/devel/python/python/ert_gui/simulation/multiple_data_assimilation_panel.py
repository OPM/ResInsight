#  Copyright (C) 2016 Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify it under the
#  terms of the GNU General Public License as published by the Free Software
#  Foundation, either version 3 of the License, or (at your option) any later
#  version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
#  A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.

from PyQt4.QtCore import Qt, QMargins
from PyQt4.QtGui import QFormLayout, QToolButton, QHBoxLayout, QLabel
from ert_gui.ide.keywords.definitions import RangeStringArgument, ProperNameFormatArgument
from ert_gui.models.connectors import EnsembleSizeModel
from ert_gui.models.connectors.init import CaseSelectorModel
from ert_gui.models.connectors.run import ActiveRealizationsModel, MultipleDataAssimilation,\
    TargetCaseFormatModel, AnalysisModuleModel, RunPathModel
from ert_gui.models.mixins.connectorless import DefaultPathModel
from ert_gui.models.mixins.connectorless import StringModel

from ert_gui.simulation import SimulationConfigPanel, AnalysisModuleVariablesPanel
from ert_gui.widgets import util
from ert_gui.widgets.active_label import ActiveLabel
from ert_gui.widgets.closable_dialog import ClosableDialog
from ert_gui.widgets.combo_choice import ComboChoice
from ert_gui.widgets.text_or_file import TextOrFile
from ert_gui.widgets.string_box import StringBox
from ert_gui.widgets.path_chooser import PathChooser

# For custom dialog box stuff
from ert_gui.models.mixins.connectorless import DefaultPathModel, DefaultNameFormatModel, StringModel
from ert_gui.ide.keywords.definitions import ProperNameFormatArgument, NumberListStringArgument

class MultipleDataAssimilationPanel(SimulationConfigPanel):
    def __init__(self):
        SimulationConfigPanel.__init__(self, MultipleDataAssimilation())

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

        target_case_format_model = TargetCaseFormatModel()
        self.target_case_format_field = StringBox(target_case_format_model, "Target case format", "config/simulation/target_case_format")
        self.target_case_format_field.setValidator(ProperNameFormatArgument())
        layout.addRow(self.target_case_format_field.getLabel(), self.target_case_format_field)

        iterated_target_case_format_model = DefaultNameFormatModel(())
        iterated_target_case_format_box = StringBox(iterated_target_case_format_model, "Target case format", "config/simulation/iterated_target_case_format")
        iterated_target_case_format_box.setValidator(ProperNameFormatArgument())

        self.option_widget = TextOrFile(self.getSimulationModel().setWeights)
        layout.addRow("Relative Weights:", self.option_widget)
        layout.addRow('Note:',
                      QLabel("Example Custom Relative Weights: '8,4,2,1'\n"
                             "This means MDA-ES will half the weight\n"
                             "applied to the Observation Errors from one\n"
                             "iteration to the next across 4 iterations."))

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

        self.target_case_format_field.validationChanged.connect(self.simulationConfigurationChanged)
        self.active_realizations_field.validationChanged.connect(self.simulationConfigurationChanged)
        self.option_widget.validationChanged.connect(self.simulationConfigurationChanged)

        self.setLayout(layout)

    def isConfigurationValid(self):
        return self.target_case_format_field.isValid() and self.active_realizations_field.isValid() and self.option_widget.isValid()

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
