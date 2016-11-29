#  Copyright (C) 2016  Statoil ASA, Norway.
#   
#  The file 'stringbox.py' is part of ERT - Ensemble based Reservoir Tool.
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


from PyQt4.QtGui import QPalette, QLineEdit
from ert_gui.ertwidgets import ValidationSupport, addHelpToWidget
from ert_gui.ertwidgets.models.valuemodel import ValueModel


class StringBox(QLineEdit):
    """StringBox shows a string. The data structure expected and sent to the getter and setter is a string."""

    def __init__(self, model, help_link="", default_string="", continuous_update=False):
        """
        :type model: ValueModel
        :type help_link: str
        :type default_string: str
        :type continuous_update: bool
        """
        QLineEdit.__init__(self)
        addHelpToWidget(self, help_link)
        self.setMinimumWidth(250)
        self._validation = ValidationSupport(self)
        self._validator = None
        self._model = model

        self.editingFinished.connect(self.stringBoxChanged)
        self.editingFinished.connect(self.validateString)

        if continuous_update:
            self.textChanged.connect(self.stringBoxChanged)

        self.textChanged.connect(self.validateString)

        self._valid_color = self.palette().color(self.backgroundRole())
        self.setText(default_string)

        self._model.valueChanged.connect(self.modelChanged)
        self.modelChanged()

    def validateString(self):
        string_to_validate = str(self.text())

        if self._validator is not None:
            status = self._validator.validate(string_to_validate)

            palette = QPalette()
            if not status:
                palette.setColor(self.backgroundRole(), ValidationSupport.ERROR_COLOR)
                self.setPalette(palette)
                self._validation.setValidationMessage(str(status), ValidationSupport.EXCLAMATION)
            else:
                palette.setColor(self.backgroundRole(), self._valid_color)
                self.setPalette(palette)
                self._validation.setValidationMessage("")

    def emitChange(self, q_string):
        self.textChanged.emit(str(q_string))

    def stringBoxChanged(self):
        """Called whenever the contents of the editline changes."""
        text = str(self.text())
        if text == "":
            text = None

        self._model.setValue(text)

    def modelChanged(self):
        """Retrieves data from the model and inserts it into the edit line"""
        text = self._model.getValue()
        if text is None:
            text = ""

        self.setText(str(text))

    def setValidator(self, validator):
        self._validator = validator

    def setAlignment(self, flag):
        self.setAlignment(flag)

    def getValidationSupport(self):
        return self._validation

    def isValid(self):
        return self._validation.isValid()



