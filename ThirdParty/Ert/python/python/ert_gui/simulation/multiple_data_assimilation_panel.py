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

from PyQt4.QtGui import QFormLayout, QLabel

from ert_gui.ertwidgets import addHelpToWidget, CaseSelector, ActiveLabel, AnalysisModuleSelector
from ert_gui.ertwidgets.models.activerealizationsmodel import ActiveRealizationsModel
from ert_gui.ertwidgets.models.ertmodel import getRealizationCount, getRunPath
from ert_gui.ertwidgets.models.targetcasemodel import TargetCaseModel
from ert_gui.ertwidgets.models.valuemodel import ValueModel
from ert_gui.ertwidgets.stringbox import StringBox
from ert_gui.ide.keywords.definitions import NumberListStringArgument, RangeStringArgument, ProperNameFormatArgument
from ert_gui.simulation import SimulationConfigPanel
from ert_gui.simulation.models import MultipleDataAssimilation


class MultipleDataAssimilationPanel(SimulationConfigPanel):
    def __init__(self):
        SimulationConfigPanel.__init__(self, MultipleDataAssimilation())

        layout = QFormLayout()

        case_selector = CaseSelector()
        layout.addRow("Current case:", case_selector)

        run_path_label = QLabel("<b>%s</b>" % getRunPath())
        addHelpToWidget(run_path_label, "config/simulation/runpath")
        layout.addRow("Runpath:", run_path_label)

        number_of_realizations_label = QLabel("<b>%d</b>" % getRealizationCount())
        addHelpToWidget(number_of_realizations_label, "config/ensemble/num_realizations")
        layout.addRow(QLabel("Number of realizations:"), number_of_realizations_label)

        self._target_case_format_model = TargetCaseModel(format_mode=True)
        self._target_case_format_field = StringBox(self._target_case_format_model, "config/simulation/target_case_format")
        self._target_case_format_field.setValidator(ProperNameFormatArgument())
        layout.addRow("Target case format:", self._target_case_format_field)

        self._createInputForWeights(layout)

        self._analysis_module_selector = AnalysisModuleSelector(iterable=False, help_link="config/analysis/analysis_module")
        layout.addRow("Analysis Module:", self._analysis_module_selector)


        self._active_realizations_model = ActiveRealizationsModel()
        self._active_realizations_field = StringBox(self._active_realizations_model, "config/simulation/active_realizations")
        self._active_realizations_field.setValidator(RangeStringArgument(getRealizationCount()))
        layout.addRow("Active realizations", self._active_realizations_field)


        self._target_case_format_field.getValidationSupport().validationChanged.connect(self.simulationConfigurationChanged)
        self._active_realizations_field.getValidationSupport().validationChanged.connect(self.simulationConfigurationChanged)
        self._relative_iteration_weights_box.getValidationSupport().validationChanged.connect(self.simulationConfigurationChanged)

        self.setLayout(layout)

    def _createInputForWeights(self, layout):
        relative_iteration_weights_model = ValueModel(self.getSimulationModel().getWeights())
        self._relative_iteration_weights_box = StringBox(relative_iteration_weights_model, help_link="config/simulation/iteration_weights", continuous_update=True)
        self._relative_iteration_weights_box.setValidator(NumberListStringArgument())
        layout.addRow("Relative Weights:", self._relative_iteration_weights_box)

        relative_iteration_weights_model.valueChanged.connect(self.getSimulationModel().setWeights)

        normalized_weights_model = ValueModel()
        normalized_weights_widget = ActiveLabel(normalized_weights_model, help_link="config/simulation/iteration_weights")
        layout.addRow('Normalized weights:', normalized_weights_widget)

        def updateVisualizationOfNormalizedWeights():
            if self._relative_iteration_weights_box.isValid():
                weights = MultipleDataAssimilation.parseWeights(relative_iteration_weights_model.getValue())
                normalized_weights = MultipleDataAssimilation.normalizeWeights(weights)
                normalized_weights_model.setValue(", ".join("%.2f" % x for x in normalized_weights))
            else:
                normalized_weights_model.setValue("The weights are invalid!")

        self._relative_iteration_weights_box.getValidationSupport().validationChanged.connect(updateVisualizationOfNormalizedWeights)

        updateVisualizationOfNormalizedWeights() # To normalize the default weights

    def isConfigurationValid(self):
        return self._target_case_format_field.isValid() and self._active_realizations_field.isValid() and self._relative_iteration_weights_box.isValid()

    def toggleAdvancedOptions(self, show_advanced):
        self._active_realizations_field.setVisible(show_advanced)
        self.layout().labelForField(self._active_realizations_field).setVisible(show_advanced)

        self._analysis_module_selector.setVisible(show_advanced)
        self.layout().labelForField(self._analysis_module_selector).setVisible(show_advanced)

    def getSimulationArguments(self):
        arguments = {"active_realizations": self._active_realizations_model.getActiveRealizationsMask(),
                     "target_case": self._target_case_format_model.getValue(),
                     "analysis_module": self._analysis_module_selector.getSelectedAnalysisModuleName()
                     }
        return arguments

