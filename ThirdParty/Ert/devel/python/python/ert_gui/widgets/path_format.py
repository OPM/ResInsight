#  Copyright (C) 2011  Statoil ASA, Norway.
#
#  The file 'path_format.py' is part of ERT - Ensemble based Reservoir Tool.
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
import re
from PyQt4.QtCore import SIGNAL
from PyQt4.QtGui import QLineEdit
from ert_gui.models.mixins import BasicModelMixin
from ert_gui.widgets.helped_widget import HelpedWidget


class PathFormatChooser(HelpedWidget):
    """
    PathChooser shows, enables choosing of and validates paths.
    The data structure expected and sent to the models getValue and setValue is a string.
    """
    path_format_msg = "Must be a path format."

    #    UNDEFINED = 0
    #    REQUIRED = 1
    #    FILE = 2
    #    DIRECTORY = 4
    #    MUST_EXIST = 8
    #    EXECUTABLE = 16

    def __init__(self, model, path_label="Path", help_link=""):
        HelpedWidget.__init__(self, path_label, help_link)
        self.path_line = QLineEdit()
        #self.pathLine.setMinimumWidth(250)
        self.addWidget(self.path_line)

        self.validColor = self.path_line.palette().color(self.path_line.backgroundRole())

        self.path_line.setText("Not initialized!")
        self.editing = False

        self.connect(self.path_line, SIGNAL('editingFinished()'), self.validatePathFormat)
        self.connect(self.path_line, SIGNAL('editingFinished()'), self.contentsChanged)
        self.connect(self.path_line, SIGNAL('textChanged(QString)'), self.validatePathFormat)

        assert isinstance(model, BasicModelMixin)
        self.model = model
        self.getPathFromModel()
        model.observable().attach(BasicModelMixin.VALUE_CHANGED_EVENT, self.getPathFromModel)


    def getValidationTypeAndColor(self):
        """Returns the type of validation message and the color that should be applied"""
        if self.must_be_set:
            color = self.ERROR_COLOR
            validity_type = self.WARNING
        else:
            color = self.INVALID_COLOR
            validity_type = self.EXCLAMATION
        return validity_type, color

    def validatePathFormat(self):
        """Called whenever the path is modified"""
        palette = self.path_line.palette()

        path = self.getPathFormat().strip()

        color = self.validColor
        message = ""
        validity_type = self.WARNING

        self.valid = True

        if not re.search("%[0-9]*d", path):
            message = self.path_format_msg
            color = self.ERROR_COLOR
            self.valid = False

        self.setValidationMessage(message, validity_type)
        self.path_line.setToolTip(message)
        palette.setColor(self.path_line.backgroundRole(), color)

        self.path_line.setPalette(palette)


    def getPathFormat(self):
        """Returns the path"""
        return str(self.path_line.text())


    def isValid(self):
        """Returns the validation value"""
        return self.valid


    def contentsChanged(self):
        """Called whenever the path is changed."""
        if not self.editing:
            self.model.setValue(self.getPathFormat())

    def getPathFromModel(self):
        """Retrieves data from the model and inserts it into the edit line"""
        self.editing = True

        path = self.model.getValue()
        if path is None:
            path = ""

        self.path_line.setText("%s" % path)
        self.editing = False

