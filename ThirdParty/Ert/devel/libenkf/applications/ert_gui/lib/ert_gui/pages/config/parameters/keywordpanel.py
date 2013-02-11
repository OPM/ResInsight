#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'keywordpanel.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from parametermodels import KeywordModel
from ert_gui.widgets.helpedwidget import ContentModel

class KeywordPanel(QtGui.QFrame):
    def __init__(self, parent):
        QtGui.QFrame.__init__(self, parent)

        self.setFrameShape(QtGui.QFrame.StyledPanel)
        self.setFrameShadow(QtGui.QFrame.Plain)
        self.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)

        layout = QtGui.QFormLayout()
        layout.setLabelAlignment(QtCore.Qt.AlignRight)

        self.keywordModel = KeywordModel("")

        self.min_std = PathChooser(self, "", "config/ensemble/gen_kw_min_std", True , must_be_set = False)
        self.modelWrap(self.min_std, "min_std")
        
        self.template = PathChooser(self, "", "config/ensemble/gen_kw_template", True)
        self.modelWrap(self.template, "template")

        self.enkf_outfile = PathChooser(self, "", "config/ensemble/gen_kw_enkf_outfile", True, must_be_set=False)
        self.modelWrap(self.enkf_outfile, "enkf_outfile")

        self.init_files = PathChooser(self, "", "config/ensemble/gen_kw_init_files", True, must_be_set=False)
        self.modelWrap(self.init_files, "init_files")

        self.parameter_file = PathChooser(self, "", "config/ensemble/gen_kw_parameter_file", True, must_be_set=False)
        self.modelWrap(self.parameter_file, "parameter_file")

        layout.addRow("Parameter file:"   , self.parameter_file)
        layout.addRow("Include file:"     , self.enkf_outfile)
        layout.addRow("Template:"         , self.template)
        layout.addRow("Minimum std:"      , self.min_std)
        layout.addRow("Init files:"       , self.init_files)

        button = QtGui.QPushButton()
        button.setText("Reload")
        button.setMaximumWidth(70)
        self.connect(button, QtCore.SIGNAL('clicked()'), self._reload)

        layout.addRow("Reload files:", button)

        self.setLayout(layout)

    def _reload(self):
        self.keywordModel.emitUpdate()

    def setKeywordModel(self, keywordModel):
        self.keywordModel = keywordModel

        self.min_std.fetchContent()
        self.template.fetchContent()
        self.enkf_outfile.fetchContent()
        self.init_files.fetchContent()
        self.parameter_file.fetchContent()

    def modelWrap(self, widget, attribute):
        widget.initialize = ContentModel.emptyInitializer
        widget.setter = lambda model, value: self.keywordModel.set(attribute, value)
        widget.getter = lambda model: self.keywordModel[attribute]

