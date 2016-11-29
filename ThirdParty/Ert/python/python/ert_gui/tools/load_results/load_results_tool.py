#  Copyright (C) 2014  Statoil ASA, Norway.
#
#  The file 'load_results_tool.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert_gui.ertwidgets import resourceIcon
from ert_gui.ertwidgets.closabledialog import ClosableDialog
from ert_gui.tools import Tool
from ert_gui.tools.load_results import LoadResultsModel
from ert_gui.tools.load_results import LoadResultsPanel


class LoadResultsTool(Tool):
    def __init__(self):
        super(LoadResultsTool, self).__init__("Load results manually", "tools/load_manually", resourceIcon("ide/table_import"))
        self.__import_widget = None
        self.__dialog = None
        self.setVisible(False)




    def trigger(self):
        if self.__import_widget is None:
            self.__import_widget = LoadResultsPanel()
        self.__dialog = ClosableDialog("Load results manually", self.__import_widget, self.parent())
        self.__import_widget.setCurrectCase()
        self.__dialog.addButton("Load", self.load)
        self.__dialog.exec_()

    def load(self):
        self.__import_widget.load()
        self.__dialog.accept()

    def toggleAdvancedMode(self, advanced_mode):
        self.setVisible(advanced_mode)
        if not LoadResultsModel.isValidRunPath():
            self.setEnabled(False)


