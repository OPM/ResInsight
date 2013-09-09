#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'datapanel.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from PyQt4 import QtGui, QtCore
from ert_gui.widgets.combochoice import ComboChoice
from ert_gui.widgets.stringbox import DoubleBox
from ert_gui.widgets.pathchooser import PathChooser
from parametermodels import DataModel
import ert.ert.enums as enums
import ert_gui.widgets.helpedwidget

class DataPanel(QtGui.QFrame):

    def __init__(self, parent):
        QtGui.QFrame.__init__(self, parent)

        self.setFrameShape(QtGui.QFrame.StyledPanel)
        self.setFrameShadow(QtGui.QFrame.Plain)
        self.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)

        layout = QtGui.QFormLayout()
        layout.setLabelAlignment(QtCore.Qt.AlignRight)

        self.dataModel = DataModel("")

        self.input = ComboChoice(self, enums.gen_data_file_format.INPUT_TYPES, "", "config/ensemble/gen_data_param_init")
        self.modelWrap(self.input, "input_format")

        self.output = ComboChoice(self, enums.gen_data_file_format.OUTPUT_TYPES, "", "config/ensemble/gen_data_param_output")
        self.modelWrap(self.output, "output_format")

        self.template_file = PathChooser(self, "", "config/ensemble/gen_data_template_file", True , must_be_set=False)
        self.modelWrap(self.template_file, "template_file")

        self.template_key = PathChooser(self, "", "config/ensemble/gen_data_template_key", True , must_be_set=False)
        self.modelWrap(self.template_key, "template_key")

        self.init_file_fmt = PathChooser(self, "", "config/ensemble/gen_data_init_file_fmt", True , must_be_set=False)
        self.modelWrap(self.init_file_fmt, "init_file_fmt")


        self.file_generated_by_enkf = PathChooser(self, "", "config/ensemble/gen_data_file_generated_by_enkf", True, must_be_set=False)
        self.modelWrap(self.file_generated_by_enkf, "enkf_outfile")

        self.file_loaded_by_enkf = PathChooser(self, "", "config/ensemble/gen_data_file_loaded_by_enkf", True, must_be_set=False)
        self.modelWrap(self.file_loaded_by_enkf, "enkf_infile")

        self.min_std = PathChooser(self, "", "config/ensemble/gen_data_min_std", True, must_be_set=False)
        self.modelWrap(self.min_std, "min_std")

        layout.addRow("Input:", self.input)
        layout.addRow("Output:", self.output)
        layout.addRow("Template file:", self.template_file)
        layout.addRow("Template key:", self.template_key)
        layout.addRow("Init files:", self.init_file_fmt)
        layout.addRow("Include file:", self.file_generated_by_enkf)
        layout.addRow("Min. std.:", self.min_std)
        layout.addRow("File loaded by EnKF:", self.file_loaded_by_enkf)

        button = QtGui.QPushButton()
        button.setText("Reload")
        button.setMaximumWidth(70)
        self.connect(button, QtCore.SIGNAL('clicked()'), self._reload)

        layout.addRow("Reload template:", button)

        self.setLayout(layout)

    def _reload(self):
        self.dataModel.emitUpdate()

    def modelWrap(self, widget, attribute):
        widget.initialize = ert_gui.widgets.helpedwidget.ContentModel.emptyInitializer
        widget.setter = lambda model, value: self.dataModel.set(attribute, value)
        widget.getter = lambda model: self.dataModel[attribute]

    def setDataModel(self, dataModel):
        self.dataModel = dataModel

        self.input.fetchContent()
        self.output.fetchContent()
        self.template_file.fetchContent()
        self.template_key.fetchContent()
        self.init_file_fmt.fetchContent()
        self.file_generated_by_enkf.fetchContent()
        self.file_loaded_by_enkf.fetchContent()
        self.min_std.fetchContent()
