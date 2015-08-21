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
from PyQt4.QtCore import Qt
from PyQt4.QtGui import QPalette
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

        self.valid_color = self.box_string.palette().color(self.box_string.backgroundRole())
        self.box_string.setText(defaultString)
        self.__validator = None

        assert isinstance(model, BasicModelMixin)
        self.model = model
        self.model.observable().attach(BasicModelMixin.VALUE_CHANGED_EVENT, self.modelChanged)
        self.modelChanged()

    def validateString(self):
        """Override this to provide validation of the contained string. NOT SUPPORTED YET!"""
        string_to_validate = str(self.box_string.text())

        if self.__validator is not None:
            status = self.__validator.validate(string_to_validate)

            palette = QPalette()
            if not status:
                self.setValidationMessage(str(status), HelpedWidget.EXCLAMATION)
                palette.setColor(self.box_string.backgroundRole(), self.ERROR_COLOR)
                self.box_string.setPalette(palette)
            else:
                self.setValidationMessage("")
                palette.setColor(self.box_string.backgroundRole(), self.valid_color)
                self.box_string.setPalette(palette)


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

    def setValidator(self, validator):
        self.__validator = validator

    def setAlignment(self, flag):
        self.box_string.setAlignment(flag)



