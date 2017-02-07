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
import os
import sys
from PyQt4.QtCore import QDir
from PyQt4.QtGui import QMessageBox
from ert.enkf import EnkfFieldFileFormatEnum
from ert_gui.tools.export import ExportModel, ExportKeywordModel


class Exporter():
    def __init__(self):
        self.__export_keyword_model = ExportKeywordModel()

    def runExport(self, values):
        keyword = values["keyword"]
        file_name = self.createExportFileNameMask(keyword, values["selected_case"], values["report_step"], values["path"])

        if self.__export_keyword_model.isFieldKw(keyword):
            self.exportField(keyword, file_name, values["iactive"], values["file_type_key"], values["report_step"], values["selected_case"])
        elif self.__export_keyword_model.isGenKw(keyword):
            self.exportGenKw(keyword, file_name, values["iactive"], values["file_type_key"], values["report_step"], values["selected_case"])
        elif self.__export_keyword_model.isGenParamKw(keyword) or self.__export_keyword_model.isGenDataKw(keyword):
            self.exportGenData(keyword, file_name, values["iactive"], values["file_type_key"], values["report_step"], values["selected_case"])
        else:
            sys.stderr.write('** WARNING: Cannot export unknown keyword type "%s".\n' % keyword)

    def exportField(self, keyword, file_name, iactive, file_type_key, report_step, selected_case):
        if file_type_key == "Eclipse GRDECL":
            file_type = EnkfFieldFileFormatEnum.ECL_GRDECL_FILE
        else:
            file_type = EnkfFieldFileFormatEnum.RMS_ROFF_FILE

        result = ExportModel().exportField(keyword, file_name, iactive, file_type, report_step, selected_case)
        if not result:
            QMessageBox.warning(self, "Warning", '''Something did not work!''', QMessageBox.Ok)

    def exportGenData(self, keyword, file_name, iactive, file_type_key, report_step, selected_case):
        ExportModel().exportGenData(keyword, file_name, iactive, file_type_key, report_step, selected_case)

    def exportGenKw(self, keyword, file_name, iactive, file_type_key, report_step, selected_case):
        ExportModel().exportGenKw(keyword, file_name, iactive, file_type_key, report_step, selected_case)

    def createExportFileNameMask(self, keyword, current_case, report_step, path):
        impl_type = None

        if self.__export_keyword_model.isFieldKw(keyword):
            impl_type = self.__export_keyword_model.getImplementationType(keyword)
        elif self.__export_keyword_model.isGenDataKw(keyword):
            impl_type = "Gen_Data"
        elif self.__export_keyword_model.isGenKw(keyword):
            impl_type = "Gen_Kw"
        elif self.__export_keyword_model.isGenParamKw(keyword):
            impl_type = "Gen_Param"

        path = os.path.join(str(path), str(current_case), str(impl_type), str(keyword))

        if self.__export_keyword_model.isGenDataKw(keyword):
            path = path + "_" + str(report_step)

        if not QDir(path).exists():
            os.makedirs(path)

        return path
