#  Copyright (C) 2011  Statoil ASA, Norway.
#
#  The file 'export_model.py' is part of ERT - Ensemble based Reservoir Tool.
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

from __future__ import print_function
import os.path
from ert.enkf import EnkfConfigNode, EnkfNode, EnkfFieldFileFormatEnum, ErtImplType
from ert.enkf import GenKw, GenDataFileType, GenData, NodeId
from ert_gui import ERT


class ExportModel(object):
    def __init__(self):
        super(ExportModel, self).__init__()

    def exportField(self, keyword, path, iactive, file_type, report_step, selected_case):
        """
        @type keyword: str
        @type path: str
        @type iactive: BoolVector
        @type file_type: EnkfFieldFileFormatEnum
        @type report_step: int
        @type selected_case: str
        """

        fs = ERT.ert.getEnkfFsManager().getFileSystem(selected_case)
        if file_type == EnkfFieldFileFormatEnum.ECL_GRDECL_FILE:
            extension = ".grdecl"
        elif file_type == EnkfFieldFileFormatEnum.RMS_ROFF_FILE:
            extension = ".roff"

        iens_list = iactive.createActiveList()
        path_fmt = os.path.join(path, keyword + "_%d" + extension)
        config_node = ERT.ert.ensembleConfig()[keyword]
        init_file = ERT.ert.fieldInitFile(config_node)
        if init_file:
            print('Using init file:%s' % init_file)
        EnkfNode.exportMany(config_node, path_fmt, fs, iens_list, file_type=file_type, arg=init_file)
        return True

    def exportGenKw(self, keyword, path, iactive, file_type, report_step, selected_case):
        """
        @type keyword: str
        @type path: str
        @type iactive: BoolVector
        @type file_type: EnkfFieldFileFormatEnum
        @type report_step: int
        @type selected_case: str
        """
        enkf_config_node = ERT.ert.ensembleConfig().getNode(keyword)
        assert isinstance(enkf_config_node, EnkfConfigNode)
        node = EnkfNode(enkf_config_node)
        fs = ERT.ert.getEnkfFsManager().getFileSystem(selected_case)

        for index, value in enumerate(iactive):
            if value:
                if node.tryLoad(fs, NodeId(report_step, index)):
                    gen_kw = GenKw.createCReference(node.valuePointer())
                    filename = str(path + "/" + keyword + "_{0}").format(index)
                    if file_type == "Parameter list":
                        filename += ".txt"
                        gen_kw.exportParameters(filename)
                    else:
                        filename += ".inc"
                        gen_kw.exportTemplate(filename)

    def exportGenData(self, keyword, path, iactive, file_type, report_step, selected_case):
        """
        @type keyword: str
        @type path: str
        @type iactive: BoolVector
        @type file_type: EnkfFieldFileFormatEnum
        @type report_step: int
        @type selected_case: str
        """
        fs = ERT.ert.getEnkfFsManager().getFileSystem(selected_case)
        config_node = ERT.ert.ensembleConfig().getNode(keyword)
        gen_data_config_node = config_node.getDataModelConfig()

        export_type = gen_data_config_node.getOutputFormat()
        if export_type == GenDataFileType.GEN_DATA_UNDEFINED:
            export_type = gen_data_config_node.getInputFormat()

        node = EnkfNode(config_node)

        for index, active in enumerate(iactive):
            if active:
                node_id = NodeId(int(report_step), index)

                if node.tryLoad(fs, node_id):
                    gen_data = node.asGenData()

                    filename = str(path + "/" + keyword + "_{0}").format(index) + ".txt"
                    gen_data.export(filename, export_type, None)
