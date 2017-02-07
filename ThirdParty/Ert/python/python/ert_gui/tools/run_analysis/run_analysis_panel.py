#  Copyright (C) 2016  Statoil ASA, Norway.
#
#  The file 'run_analysis_panel.py' is part of ERT - Ensemble based Reservoir Tool.
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
from PyQt4.QtGui import QWidget, QFormLayout, QLineEdit

from ert_gui.ertwidgets.analysismoduleselector import AnalysisModuleSelector
from ert_gui.ertwidgets.caseselector import CaseSelector


class RunAnalysisPanel(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self.setWindowTitle("Run Analysis")
        self.activateWindow()

        self.analysis_module = AnalysisModuleSelector(load_all = True, help_link = "config/analysis/analysis_module")
        self.target_case_text = QLineEdit()
        self.source_case_selector = CaseSelector(update_ert = False)

        layout = QFormLayout()
        layout.addRow("Analysis", self.analysis_module)
        layout.addRow("Target case", self.target_case_text)
        layout.addRow("Source case", self.source_case_selector)
        self.setLayout(layout)

    def target_case(self):
        return str(self.target_case_text.text())

    def source_case(self):
        return str(self.source_case_selector.currentText())

    def module(self):
        return self.analysis_module.getSelectedAnalysisModuleName()
