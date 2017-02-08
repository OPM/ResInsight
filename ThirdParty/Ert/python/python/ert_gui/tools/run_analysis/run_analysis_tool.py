#  Copyright (C) 2016  Statoil ASA, Norway.
#
#  The file 'run_analysis_tool.py' is part of ERT - Ensemble based Reservoir Tool.
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

from PyQt4.QtGui import QMessageBox

from ert_gui import ERT
from ert.enkf import ESUpdate
from ert_gui.ertwidgets import resourceIcon
from ert_gui.ertwidgets.closabledialog import ClosableDialog
from ert_gui.tools import Tool
from ert_gui.tools.run_analysis import RunAnalysisPanel


class RunAnalysisTool(Tool):
    def __init__(self):
        super(RunAnalysisTool, self).__init__("Run Analysis", "tools/run_analysis", resourceIcon("ide/table_import"))
        self._run_widget = None
        self._dialog = None
        self._selected_case_name = None
        self.setVisible(False)


    def trigger(self):
        if self._run_widget is None:
            self._run_widget = RunAnalysisPanel()
        self._dialog = ClosableDialog("Run Analysis", self._run_widget, self.parent())
        self._dialog.addButton("Run", self.run)
        self._dialog.exec_()

    def run(self):
        target = self._run_widget.target_case()
        source = self._run_widget.source_case()

        ert = ERT.ert
        fs_manager = ert.getEnkfFsManager() 
        es_update = ESUpdate(ert)

        target_fs = fs_manager.getFileSystem(target)
        source_fs = fs_manager.getFileSystem(source)
        success = es_update.smootherUpdate( source_fs , target_fs )

        if not success:
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Warning)
            msg.setWindowTitle("Run Analysis")
            msg.setText("Unable to run analysis for case '%s'." % source)
            msg.setStandardButtons(QMessageBox.Ok)
            msg.exec_()
            return

        ERT.ertChanged.emit()
        self._dialog.accept()

    def toggleAdvancedMode(self, advanced_mode):
        self.setVisible(advanced_mode)
