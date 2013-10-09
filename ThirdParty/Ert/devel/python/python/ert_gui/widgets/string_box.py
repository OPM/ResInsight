#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'string_box.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert_gui.models.mixins import BasicModelMixin
from ert_gui.widgets.helped_widget import HelpedWidget


class StringBox(HelpedWidget):
    """StringBox shows a string. The data structure expected and sent to the getter and setter is a string."""

    def __init__(self, model, path_label="String", help_link="", defaultString=""):
        HelpedWidget.__init__(self, path_label, help_link)

        self.box_string = QtGui.QLineEdit()
        self.connect(self.box_string, QtCore.SIGNAL('editingFinished()'), self.validateString)
        self.connect(self.box_string, QtCore.SIGNAL('editingFinished()'), self.stringBoxChanged)
        self.connect(self.box_string, QtCore.SIGNAL('textChanged(QString)'), self.validateString)
        self.addWidget(self.box_string)

        self.box_string.setText(defaultString)

        assert isinstance(model, BasicModelMixin)
        self.model = model
        self.model.observable().attach(BasicModelMixin.VALUE_CHANGED_EVENT, self.modelChanged)
        self.modelChanged()

    def validateString(self):
        """Override this to provide validation of the contained string. NOT SUPPORTED YET!"""
        stringToValidate = self.box_string.text()
        #todo implement validation possibility


    def stringBoxChanged(self):
        """Called whenever the contents of the editline changes."""
        text = str(self.box_string.text())
        if text == "":
            text = None

        self.model.setValue(text)

    def modelChanged(self):
        """Retrieves data from the model and inserts it into the edit line"""
        text = self.model.getValue()
        if text is None:
            text = ""

        self.box_string.setText(str(text))




