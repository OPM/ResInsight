#  Copyright (C) 2014  Statoil ASA, Norway.
#
#  The file 'export_panel.py' is part of ERT - Ensemble based Reservoir Tool.
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
from PyQt4.QtCore import QDir, pyqtSignal
from PyQt4.QtGui import QFormLayout, QWidget, QLineEdit, QToolButton, QHBoxLayout, QFileDialog, QComboBox

from ert_gui.ertwidgets.models.activerealizationsmodel import ActiveRealizationsModel
from ert_gui.ertwidgets.models.all_cases_model import AllCasesModel
from ert_gui.ertwidgets.models.ertmodel import getCurrentCaseName
from ert_gui.ertwidgets.stringbox import StringBox
from ert_gui.ide.keywords.definitions import RangeStringArgument
from ert_gui.tools.export import ExportKeywordModel


class ExportPanel(QWidget):
    updateExportButton = pyqtSignal(str, bool)
    runExport = pyqtSignal(dict)

    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.setMinimumWidth(500)
        self.setMinimumHeight(200)
        self._dynamic = False

        self.setWindowTitle("Export data")
        self.activateWindow()

        layout = QFormLayout()
        current_case = getCurrentCaseName()

        self._case_model = AllCasesModel()
        self._case_combo = QComboBox()
        self._case_combo.setSizeAdjustPolicy(QComboBox.AdjustToMinimumContentsLength)
        self._case_combo.setMinimumContentsLength(20)
        self._case_combo.setModel(self._case_model)
        self._case_combo.setCurrentIndex(self._case_model.indexOf(current_case))
        layout.addRow("Select case:", self._case_combo)

        self._export_keyword_model = ExportKeywordModel()

        self._kw_model = self._export_keyword_model.getKeyWords()
        self._keywords = QComboBox()
        self._keywords.addItems(self._kw_model)
        layout.addRow("Select keyword:", self._keywords)

        self._active_realizations_model = ActiveRealizationsModel()
        self._active_realizations_field = StringBox(self._active_realizations_model, "config/simulation/active_realizations")
        self._active_realizations_field.setValidator(RangeStringArgument())
        self._active_realizations_field.getValidationSupport().validationChanged.connect(self.validateExportDialog)
        layout.addRow("Active realizations:", self._active_realizations_field)

        file_name_button = QToolButton()
        file_name_button.setText("Browse")
        file_name_button.clicked.connect(self.selectFileDirectory)

        self._defaultPath = QDir.currentPath() + "/export"
        self._file_name = QLineEdit()
        self._file_name.setEnabled(False)
        self._file_name.setText(self._defaultPath)
        self._file_name.textChanged.connect(self.validateExportDialog)
        self._file_name.setMinimumWidth(250)

        file_name_layout = QHBoxLayout()
        file_name_layout.addWidget(self._file_name)
        file_name_layout.addWidget(file_name_button)
        layout.addRow("Select directory to save files to:", file_name_layout)

        self._gen_kw_file_types = ["Parameter list", "Template based"]
        self._field_kw_file_types = ["Eclipse GRDECL", "RMS roff"]
        self._gen_data_file_types = ["Gen data"]

        self._file_type_model = self._field_kw_file_types
        self._file_type_combo = QComboBox()
        self._file_type_combo.setSizeAdjustPolicy(QComboBox.AdjustToContents)
        self._file_type_combo.addItems(self._file_type_model)
        layout.addRow("Select file format:", self._file_type_combo)

        self._report_step = QLineEdit()
        layout.addRow("Report step:", self._report_step)

        self._gen_data_report_step_model = []
        self._gen_data_report_step = QComboBox()
        layout.addRow("Report step:", self._gen_data_report_step)

        self.setLayout(layout)
        self._keywords.currentIndexChanged.connect(self.keywordSelected)
        self.keywordSelected()

    def selectFileDirectory(self):
        directory = QFileDialog().getExistingDirectory(self, "Directory", self._file_name.text(), QFileDialog.ShowDirsOnly)
        if str(directory).__len__() > 0:
            self._file_name.setText(str(directory))

    def updateFileExportType(self, keyword):
        self._file_type_combo.clear()
        if self._export_keyword_model.isGenKw(keyword):
            self._file_type_model = self._gen_kw_file_types
        elif self._export_keyword_model.isGenParamKw(keyword):
            self._file_type_model = self._gen_data_file_types
        elif self._export_keyword_model.isGenDataKw(keyword):
            self._file_type_model = self._gen_data_file_types
        else:
            self._file_type_model = self._field_kw_file_types

        self._file_type_combo.addItems(self._file_type_model)

    def export(self):
        keyword = self._kw_model[self._keywords.currentIndex()]
        report_step = self.getReportStep(keyword)
        all_cases = self._case_model.getAllItems()
        selected_case = all_cases[self._case_combo.currentIndex()]
        path = self._file_name.text()
        iactive = self._active_realizations_model.getActiveRealizationsMask()
        file_type_key = self._file_type_model[self._file_type_combo.currentIndex()]
        values = {"keyword": keyword,
                  "report_step": report_step,
                  "iactive": iactive,
                  "file_type_key": file_type_key,
                  "path": path,
                  "selected_case": selected_case}
        self.runExport.emit(values)

    def getReportStep(self, key):
        report_step = 0
        if self._dynamic:
            report_step = self._report_step.text()

        if self._export_keyword_model.isGenParamKw(key):
            return report_step

        if self._export_keyword_model.isGenDataKw(key):
            lst = self._gen_data_report_step_model
            idx = self._gen_data_report_step.currentIndex()
            if lst and len(lst) > idx:
                report_step = lst[idx]
            else:
                raise IndexError('No such model step: %d.  Valid range: [0, %d)' % (idx, len(lst)))

        return report_step

    def keywordSelected(self):
        key = self._kw_model[self._keywords.currentIndex()]
        self.updateFileExportType(key)
        self._dynamic = False
        if self._export_keyword_model.isFieldKw(key):
            self._dynamic = self._export_keyword_model.isDynamicField(key)

        self._report_step.setVisible(self._dynamic)
        self.layout().labelForField(self._report_step).setVisible(self._dynamic)

        self._gen_data_report_step.setVisible(self._export_keyword_model.isGenDataKw(key))
        self.layout().labelForField(self._gen_data_report_step).setVisible(self._export_keyword_model.isGenDataKw(key))

        if self._export_keyword_model.isGenDataKw(key):
            data = self._export_keyword_model.getGenDataReportSteps(key)
            self._gen_data_report_step_model = data
            self._gen_data_report_step.clear()
            self._gen_data_report_step.addItems(self._gen_data_report_step_model)

    def setSelectedCase(self, selected_case):
        self._case_combo.setCurrentIndex(self._case_model.indexOf(selected_case))

    def validateExportDialog(self):
        validRealizations = False
        if self._active_realizations_field.isValid():
            validRealizations = True

        path = str(self._file_name.text())
        validPath = len(path) > 0

        if validRealizations and validPath:
            self.updateExportButton.emit("export", True)
        else:
            self.updateExportButton.emit("export", False)
