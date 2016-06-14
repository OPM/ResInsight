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
from PyQt4.QtGui import  QFormLayout, QWidget, QLineEdit, QToolButton, QHBoxLayout, QFileDialog, QComboBox
from ert_gui.ide.keywords.definitions import RangeStringArgument
from ert_gui.models.connectors import EnsembleSizeModel
from ert_gui.models.connectors.export import ExportKeywordModel
from ert_gui.models.connectors.init import CaseSelectorModel
from ert_gui.tools.export import ExportRealizationsModel
from ert_gui.models.qt.all_cases_model import AllCasesModel
from ert_gui.widgets.string_box import StringBox


class ExportPanel(QWidget):

    updateExportButton = pyqtSignal(str, bool)
    runExport = pyqtSignal(dict)

    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.setMinimumWidth(500)
        self.setMinimumHeight(200)
        self.__dynamic = False

        self.setWindowTitle("Export data")
        self.activateWindow()

        layout = QFormLayout()
        current_case = CaseSelectorModel().getCurrentChoice()

        self.__case_model = AllCasesModel()
        self.__case_combo = QComboBox()
        self.__case_combo.setSizeAdjustPolicy(QComboBox.AdjustToMinimumContentsLength)
        self.__case_combo.setMinimumContentsLength(20)
        self.__case_combo.setModel(self.__case_model)
        self.__case_combo.setCurrentIndex(self.__case_model.indexOf(current_case))
        layout.addRow("Select case:",self.__case_combo)

        self.__export_keyword_model = ExportKeywordModel()

        self.__kw_model = self.__export_keyword_model.getKeyWords()
        self.__keywords = QComboBox()
        self.__keywords.addItems(self.__kw_model)
        layout.addRow("Select keyword:",self.__keywords)

        self.__active_realizations_model = ExportRealizationsModel(EnsembleSizeModel().getValue())
        self.__active_realizations_field = StringBox(self.__active_realizations_model, "Active realizations", "config/simulation/active_realizations")
        self.__active_realizations_field.setValidator(RangeStringArgument())
        self.__active_realizations_field.validationChanged.connect(self.validateExportDialog)
        layout.addRow(self.__active_realizations_field.getLabel(), self.__active_realizations_field)

        file_name_button= QToolButton()
        file_name_button.setText("Browse")
        file_name_button.clicked.connect(self.selectFileDirectory)

        self.__defaultPath = QDir.currentPath()+"/export"
        self.__file_name = QLineEdit()
        self.__file_name.setEnabled(False)
        self.__file_name.setText(self.__defaultPath)
        self.__file_name.textChanged.connect(self.validateExportDialog)
        self.__file_name.setMinimumWidth(250)

        file_name_layout = QHBoxLayout()
        file_name_layout.addWidget(self.__file_name)
        file_name_layout.addWidget(file_name_button)
        layout.addRow("Select directory to save files to:", file_name_layout)

        self.__gen_kw_file_types = ["Parameter list", "Template based"]
        self.__field_kw_file_types = ["Eclipse GRDECL", "RMS roff"]
        self.__gen_data_file_types = ["Gen data"]

        self.__file_type_model = self.__field_kw_file_types
        self.__file_type_combo = QComboBox()
        self.__file_type_combo.setSizeAdjustPolicy(QComboBox.AdjustToContents)
        self.__file_type_combo.addItems(self.__file_type_model)
        layout.addRow("Select file format:",self.__file_type_combo)

        self.__report_step = QLineEdit()
        layout.addRow("Report step:", self.__report_step)

        self.__gen_data_report_step_model=[]
        self.__gen_data_report_step = QComboBox()
        layout.addRow("Report step:", self.__gen_data_report_step)

        self.setLayout(layout)
        self.__keywords.currentIndexChanged.connect(self.keywordSelected)
        self.keywordSelected()

    def selectFileDirectory(self):
        directory = QFileDialog().getExistingDirectory(self, "Directory", self.__file_name.text(), QFileDialog.ShowDirsOnly)
        if str(directory).__len__() > 0:
            self.__file_name.setText(str(directory))

    def updateFileExportType(self, keyword):
        self.__file_type_combo.clear()
        if self.__export_keyword_model.isGenKw(keyword):
            self.__file_type_model = self.__gen_kw_file_types
        elif self.__export_keyword_model.isGenParamKw(keyword):
            self.__file_type_model = self.__gen_data_file_types
        elif self.__export_keyword_model.isGenDataKw(keyword):
            self.__file_type_model = self.__gen_data_file_types
        else:
            self.__file_type_model = self.__field_kw_file_types

        self.__file_type_combo.addItems(self.__file_type_model)

    def export(self):
        keyword = self.__kw_model[self.__keywords.currentIndex()]
        report_step = self.getReportStep(keyword)
        all_cases = self.__case_model.getAllItems()
        selected_case  = all_cases[self.__case_combo.currentIndex()]
        path = self.__file_name.text()
        iactive = self.__active_realizations_model.getActiveRealizationsMask()
        file_type_key = self.__file_type_model[self.__file_type_combo.currentIndex()]
        values = {"keyword":keyword, "report_step":report_step, "iactive":iactive,"file_type_key":file_type_key, "path":path , "selected_case" : selected_case}
        self.runExport.emit(values)
        
    

    def getReportStep(self, key):
        report_step = 0
        if self.__dynamic:
            report_step = self.__report_step.text()

        if self.__export_keyword_model.isGenParamKw(key):
            return report_step

        if self.__export_keyword_model.isGenDataKw(key):
            report_step = self.__gen_data_report_step_model[self.__gen_data_report_step.currentIndex()]

        return report_step


    def keywordSelected(self):
        key = self.__kw_model[self.__keywords.currentIndex()]
        self.updateFileExportType(key)
        self.__dynamic = False
        if self.__export_keyword_model.isFieldKw(key):
            self.__dynamic = self.__export_keyword_model.isDynamicField(key)

        self.__report_step.setVisible(self.__dynamic)
        self.layout().labelForField(self.__report_step).setVisible(self.__dynamic)

        self.__gen_data_report_step.setVisible(self.__export_keyword_model.isGenDataKw(key))
        self.layout().labelForField(self.__gen_data_report_step).setVisible(self.__export_keyword_model.isGenDataKw(key))

        if self.__export_keyword_model.isGenDataKw(key):
            data = self.__export_keyword_model.getGenDataReportSteps(key)
            self.__gen_data_report_step_model = data
            self.__gen_data_report_step.clear()
            self.__gen_data_report_step.addItems(self.__gen_data_report_step_model)

    def setSelectedCase(self, selected_case):
        self.__case_combo.setCurrentIndex(self.__case_model.indexOf(selected_case))

    def validateExportDialog(self):
        validRealizations = False
        if self.__active_realizations_field.isValid():
            validRealizations = True

        path = str(self.__file_name.text())
        validPath = len(path) > 0

        if validRealizations and validPath:
            self.updateExportButton.emit("export", True)
        else:
            self.updateExportButton.emit("export", False)
