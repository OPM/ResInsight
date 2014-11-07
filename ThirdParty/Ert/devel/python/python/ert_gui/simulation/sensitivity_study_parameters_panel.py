#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'analysis_module_variables_panel.py' is part of ERT - Ensemble based Reservoir Tool.
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

from collections import namedtuple

from PyQt4.QtCore import Qt, QSize
from PyQt4.QtGui import QTableWidget, QHeaderView, QLabel, QTableWidgetItem

from ert_gui.ide.keywords.definitions import FloatArgument
from ert_gui.models.connectors.run import SensitivityStudyParametersModel, \
    SensivityStudyParametersConstantValueModel, SensivityStudyParametersIsIncludedModel
from ert_gui.widgets.checkbox import CheckBox
from ert_gui.widgets.string_box import StringBox
from ert.enkf.enums.ert_impl_type_enum import ErtImplType


class SensitivityStudyParametersPanel(QTableWidget):

    Column = namedtuple("Column_tuple", "index header")

    columns = {"name"        : Column(index = 0, header = "Parameter Name"),
               "is_active"   : Column(index = 1, header = "Include"),
               "const_value" : Column(index = 2, header = "Constant Value")}

    column_list = ["name", "is_active", "const_value"]

    def __init__(self, parent=None):
        model = SensitivityStudyParametersModel()
        parameters = model.getParameters()
        n_parameters = len(parameters)

        super(QTableWidget, self).__init__(n_parameters, len(self.columns), parent)
        self.verticalHeader().setResizeMode(QHeaderView.Fixed)
        self.verticalHeader().hide()

        headers = [self.columns[col_id].header for col_id in self.column_list]
        self.setHorizontalHeaderLabels(headers)

        for row in range(n_parameters):
            param_name = parameters[row]

            param_name_widget = QLabel(param_name)
            param_name_widget.setMargin(5)
            self.setCellWidget(row, self.columns["name"].index, param_name_widget)

            if (model.getParameterType(param_name) == ErtImplType.GEN_KW):
                const_value_model = SensivityStudyParametersConstantValueModel(param_name, model)
                const_value_widget = StringBox(const_value_model, "Constant value", 
                                               "config/simulation/sensitivity_parameter_constant_value")
                const_value_widget.setValidator(FloatArgument())
                const_value_widget.setAlignment(Qt.AlignRight)
                self.setCellWidget(row, self.columns["const_value"].index, const_value_widget)
            else:
                empty_item = QTableWidgetItem()
                empty_item.setFlags(empty_item.flags() ^ Qt.ItemIsEditable)
                self.setItem(row, self.columns["const_value"].index, empty_item)


            is_active_model = SensivityStudyParametersIsIncludedModel(param_name, model)
            is_active_widget = CheckBox(is_active_model, "Is included", 
                                        "config/simulation/sensitivity_parameter_is_included", show_label=False)
            self.setCellWidget(row, self.columns["is_active"].index, is_active_widget)


        self.resizeColumnsToContents()
        self.setMinimumWidth(self.sizeHint().width())

        self.blockSignals(False)

    def sizeHint(self):
        height = QTableWidget.sizeHint(self).height()
        
        width = self.horizontalHeader().length()
        
        margins = self.contentsMargins()
        width += margins.left() + margins.right()
        
        width += self.verticalScrollBar().sizeHint().width()
        
        return QSize(width, height)
        
