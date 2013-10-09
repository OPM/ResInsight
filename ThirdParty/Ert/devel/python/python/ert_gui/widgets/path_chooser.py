#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'path_chooser.py' is part of ERT - Ensemble based Reservoir Tool.
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
import re
from PyQt4.QtCore import SIGNAL, QSize
from PyQt4.QtGui import QLineEdit, QToolButton, QFileDialog
from ert_gui.models.mixins import PathModelMixin
from ert_gui.widgets.util import resourceIcon
from ert_gui.widgets.helped_widget import HelpedWidget


class PathChooser(HelpedWidget):
    """
    PathChooser shows, enables choosing of and validates paths. 
    The data structure expected and sent to the models getValue and setValue is a string.
    """

    file_does_not_exist_msg = "The specified path does not exist."
    file_is_not_executable_msg = "The specified file is not an executable."
    path_is_not_a_file_msg = "The specified path must be a file."
    required_field_msg = "A value is required."

#    UNDEFINED = 0
#    REQUIRED = 1
#    FILE = 2
#    DIRECTORY = 4
#    MUST_EXIST = 8
#    EXECUTABLE = 16

    def __init__(self, model, path_label="Path", help_link=""):
        HelpedWidget.__init__(self, path_label, help_link)

        self.__editing = True

        self.path_line = QLineEdit()

        self.addWidget(self.path_line)

        dialog_button = QToolButton(self)
        dialog_button.setIcon(resourceIcon("folder"))
        dialog_button.setIconSize(QSize(16, 16))
        self.connect(dialog_button, SIGNAL('clicked()'), self.selectDirectory)
        self.addWidget(dialog_button)

        self.valid_color = self.path_line.palette().color(self.path_line.backgroundRole())

        self.path_line.setText(os.getcwd())
        self.__editing = False

        assert isinstance(model, PathModelMixin)
        self.model = model
        model.observable().attach(PathModelMixin.PATH_CHANGED_EVENT, self.getPathFromModel)
        self.getPathFromModel()

        self.connect(self.path_line, SIGNAL('editingFinished()'), self.validatePath)
        self.connect(self.path_line, SIGNAL('editingFinished()'), self.contentsChanged)
        self.connect(self.path_line, SIGNAL('textChanged(QString)'), self.validatePath)


    def getValidationTypeAndColor(self):
        """Returns the type of validation message and the color that should be applied"""
        if self.model.pathIsRequired():
            color = self.ERROR_COLOR
            validity_type = self.WARNING
        else:
            color = self.INVALID_COLOR
            validity_type = self.EXCLAMATION
        return validity_type, color

    def validatePath(self):
        """Called whenever the path is modified"""
        palette = self.path_line.palette()

        path = self.getPath().strip()
        exists = os.path.exists(path)

        color = self.valid_color
        message = ""
        validity_type = self.WARNING

        self.valid = True

        if path == "" and self.model.pathIsRequired():
            message = self.required_field_msg
            color = self.ERROR_COLOR
            self.valid = False
        elif not exists:
            if self.model.pathMustExist():
                message = self.file_does_not_exist_msg
                self.valid = False
                validity_type, color = self.getValidationTypeAndColor()
        elif exists:
            if self.model.pathMustBeExecutable() and os.path.isfile(path) and not os.access(path, os.X_OK):
                validity_type, color = self.getValidationTypeAndColor()
                message = self.file_is_not_executable_msg
                self.valid = False
            elif self.model.pathMustBeExecutable() and not os.path.isfile(path):
                validity_type, color = self.getValidationTypeAndColor()
                message = self.path_is_not_a_file_msg
                self.valid = False
            

        self.setValidationMessage(message, validity_type)
        self.path_line.setToolTip(message)
        palette.setColor(self.path_line.backgroundRole(), color)

        self.path_line.setPalette(palette)


    def getPath(self):
        """Returns the path"""
        return str(self.path_line.text())

    def pathExists(self):
        """Returns True if the entered path exists"""
        return os.path.exists(self.getPath())
        
    def isValid(self):
        """Returns the validation value"""
        return self.valid


    def selectDirectory(self):
        """Pops up the 'select a directory' dialog"""
        self.__editing = True
        currentDirectory = self.getPath()

        #if not os.path.exists(currentDirectory):
        #    currentDirectory = "~"

        if self.model.pathMustBeAFile():
            currentDirectory = QFileDialog.getOpenFileName(self, "Select a path", currentDirectory)
        else:
            currentDirectory = QFileDialog.getExistingDirectory(self, "Select a directory", currentDirectory)

        if not currentDirectory == "":
            if not self.model.pathMustBeAbsolute():
                cwd = os.getcwd()
                match = re.match(cwd + "/(.*)", currentDirectory)
                if match:
                    currentDirectory = match.group(1)

            self.path_line.setText(currentDirectory)
            self.model.setPath(self.getPath())

        self.__editing = False


    def contentsChanged(self):
        """Called whenever the path is changed."""
        if not self.__editing:
            if self.model.pathIsRequired() and (self.model.pathMustExist() and self.pathExists()):
                # print("Pathchooser value set!")
                self.model.setPath(self.getPath())

                #todo: FIX!!!!

            # else:
            #     print("Pathchooser value not set!")


    def getPathFromModel(self):
        """Retrieves data from the model and inserts it into the edit line"""
        self.__editing = True

        path = self.model.getPath()
        if path is None:
            path = ""

        self.path_line.setText("%s" % path)
        self.__editing = False

