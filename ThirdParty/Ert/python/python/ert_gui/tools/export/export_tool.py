#  Copyright (C) 2014  Statoil ASA, Norway.
#
#  The file 'export_tool.py' is part of ERT - Ensemble based Reservoir Tool.
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
from weakref import ref

from ert_gui.ertwidgets import resourceIcon
from ert_gui.ertwidgets.closabledialog import ClosableDialog
from ert_gui.ertwidgets.models.ertmodel import getCurrentCaseName
from ert_gui.tools import Tool
from ert_gui.tools.export import ExportPanel, Exporter, ExportKeywordModel


class ExportTool(Tool):
    def __init__(self):
        super(ExportTool, self).__init__("Export Data", "tools/export", resourceIcon("ide/table_export"))
        self.__export_widget = None
        self.__dialog = None
        self.__exporter = None
        self.setEnabled(ExportKeywordModel().hasKeywords())

    def trigger(self):
        if self.__export_widget is None:
            self.__export_widget = ref(ExportPanel(self.parent()))
            self.__exporter = Exporter()
            self.__export_widget().runExport.connect(self.__exporter.runExport)

        self.__export_widget().setSelectedCase(getCurrentCaseName())
        self.__dialog = ref(ClosableDialog("Export", self.__export_widget(), self.parent()))
        self.__export_widget().updateExportButton.connect(self.__dialog().toggleButton)
        self.__dialog().addButton("Export", self.export)
        self.__dialog().show()

    def export(self):
        self.__export_widget().export()
        self.__dialog().accept()
