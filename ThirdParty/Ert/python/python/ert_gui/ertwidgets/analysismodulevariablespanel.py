#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'analysismodulevariablespanel.py' is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.
from functools import partial

from PyQt4.QtCore import QString
from PyQt4.QtGui import QDoubleSpinBox, QWidget, QFormLayout, QCheckBox, QLineEdit, QHBoxLayout, QSpinBox, QLabel

from ert_gui.ertwidgets.models.analysismodulevariablesmodel import AnalysisModuleVariablesModel


class AnalysisModuleVariablesPanel(QWidget):
    def __init__(self, analysis_module_name, parent=None):
        QWidget.__init__(self, parent)

        self._analysis_module_name = analysis_module_name

        layout = QFormLayout()
        variable_names = AnalysisModuleVariablesModel.getVariableNames(self._analysis_module_name)

        if len(variable_names) == 0:
            label = QString("No variables found to edit")
            boxlayout = QHBoxLayout()
            layout.addRow(label, boxlayout)

        else:
            analysis_module_variables_model = AnalysisModuleVariablesModel
            self.blockSignals(True)

            variable_names2 = self.sortVariables(variable_names)
            for variable_name in variable_names2:
                variable_type = analysis_module_variables_model.getVariableType(variable_name)
                variable_value = analysis_module_variables_model.getVariableValue(self._analysis_module_name,
                                                                                  variable_name)

                label_name = analysis_module_variables_model.getVariableLabelName(variable_name)
                if variable_type == bool:
                    spinner = self.createCheckBox(variable_name, variable_value, variable_type)

                elif variable_type == float:
                    spinner = self.createDoubleSpinBox(variable_name, variable_value, variable_type,
                                                       analysis_module_variables_model)

                elif variable_type == str:
                    spinner = self.createLineEdit(variable_name, variable_value, variable_type)

                elif variable_type == int:
                    spinner = self.createSpinBox(variable_name, variable_value, variable_type,
                                                 analysis_module_variables_model)

                layout.addRow(label_name, spinner)
                if variable_name == "LAMBDA0":
                    label = QLabel(
                        "<span style=\"font-size:12pt; font-weight:300;font-style:italic;\"> Initial Lambda of -1.00 signifies that the value will be calculated</span>")
                    layout.addRow(label, None)

        self.setLayout(layout)
        self.blockSignals(False)

    def sortVariables(self, variable_list):
        analysis_module_variables_model = AnalysisModuleVariablesModel
        sorted_list = ["#", "#", "#", "#", "#", "#", "#", "#", "#", "#", "#", "#", "#", "#"]
        result = []
        for name in variable_list:
            pos = analysis_module_variables_model.getVariablePosition(name)
            sorted_list.insert(pos, name)
            sorted_list.__delitem__(pos + 1)

        for item in sorted_list:
            if item != "#":
                result.append(item)

        return result

    def createSpinBox(self, variable_name, variable_value, variable_type, analysis_module_variables_model):
        spinner = QSpinBox()
        spinner.setMinimumWidth(75)
        spinner.setMaximum(analysis_module_variables_model.getVariableMaximumValue(variable_name))
        spinner.setMinimum(analysis_module_variables_model.getVariableMinimumValue(variable_name))
        spinner.setSingleStep(analysis_module_variables_model.getVariableStepValue(variable_name))
        if variable_value is not None:
            spinner.setValue(variable_value)
        spinner.valueChanged.connect(partial(self.valueChanged, variable_name, variable_type, spinner))
        return spinner

    def createLineEdit(self, variable_name, variable_value, variable_type):
        spinner = QLineEdit()
        if variable_value == "None":
            spinner.setText("")
        else:
            spinner.setText(variable_value)
        spinner.editingFinished.connect(partial(self.valueChanged, variable_name, variable_type, spinner))
        return spinner

    def createCheckBox(self, variable_name, variable_value, variable_type):
        spinner = QCheckBox()
        spinner.setChecked(variable_value)
        spinner.clicked.connect(partial(self.valueChanged, variable_name, variable_type, spinner))
        return spinner

    def createDoubleSpinBox(self, variable_name, variable_value, variable_type, analysis_module_variables_model):
        spinner = QDoubleSpinBox()
        spinner.setMinimumWidth(75)
        spinner.setMaximum(analysis_module_variables_model.getVariableMaximumValue(variable_name))
        spinner.setMinimum(analysis_module_variables_model.getVariableMinimumValue(variable_name))
        spinner.setSingleStep(analysis_module_variables_model.getVariableStepValue(variable_name))
        spinner.setValue(variable_value)
        spinner.valueChanged.connect(partial(self.valueChanged, variable_name, variable_type, spinner))
        return spinner;

    def valueChanged(self, variable_name, variable_type, variable_control):
        value = None
        if variable_type == bool:
            assert isinstance(variable_control, QCheckBox)
            value = variable_control.isChecked()
        elif variable_type == float:
            assert isinstance(variable_control, QDoubleSpinBox)
            value = variable_control.value()
        elif variable_type == str:
            assert isinstance(variable_control, QLineEdit)
            value = variable_control.text()
            value = str(value).strip()
            if len(value) == 0:
                value = None
        elif variable_type == int:
            assert isinstance(variable_control, QSpinBox)
            value = variable_control.value()

        if value is not None:
            AnalysisModuleVariablesModel.setVariableValue(self._analysis_module_name, variable_name, value)
