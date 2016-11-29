#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'pathchooser.py' is part of ERT - Ensemble based Reservoir Tool.
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
from PyQt4.QtCore import QSize
from PyQt4.QtGui import QLineEdit, QToolButton, QFileDialog, QWidget, QHBoxLayout

from ert_gui.ertwidgets import resourceIcon, addHelpToWidget
from ert_gui.ertwidgets.validationsupport import ValidationSupport


class PathChooser(QWidget):
    """
    PathChooser: shows, enables choosing of and validates paths.
    The data structure expected and sent to the models getValue and setValue is a string.
    """

    PATH_DOES_NOT_EXIST_MSG = "The specified path does not exist."
    FILE_IS_NOT_EXECUTABLE_MSG = "The specified file is not an executable."
    PATH_IS_NOT_A_FILE_MSG = "The specified path must be a file."
    PATH_IS_NOT_ABSOLUTE_MSG = "The specified path must be an absolute path."
    PATH_IS_NOT_A_DIRECTORY_MSG = "The specified path must be a directory."
    REQUIRED_FIELD_MSG = "A path is required."

#    UNDEFINED = 0
#    REQUIRED = 1
#    FILE = 2
#    DIRECTORY = 4
#    MUST_EXIST = 8
#    EXECUTABLE = 16

    def __init__(self, model, help_link=""):
        """
        :type model: ert_gui.ertwidgets.models.path_model.PathModel
        :param help_link: str
        """
        QWidget.__init__(self)
        addHelpToWidget(self, help_link)
        self._validation_support = ValidationSupport(self)

        self._editing = True

        layout = QHBoxLayout()
        layout.setMargin(0)

        self._path_line = QLineEdit()
        self._path_line.setMinimumWidth(250)

        layout.addWidget(self._path_line)

        dialog_button = QToolButton(self)
        dialog_button.setIcon(resourceIcon("ide/small/folder"))
        dialog_button.setIconSize(QSize(16, 16))
        dialog_button.clicked.connect(self.selectPath)
        layout.addWidget(dialog_button)

        self.valid_color = self._path_line.palette().color(self._path_line.backgroundRole())

        self._path_line.setText(os.getcwd())
        self._editing = False

        self._model = model
        self._model.valueChanged.connect(self.getPathFromModel)

        self._path_line.editingFinished.connect(self.validatePath)
        self._path_line.editingFinished.connect(self.contentsChanged)
        self._path_line.textChanged.connect(self.validatePath)

        self.setLayout(layout)
        self.getPathFromModel()


    def isPathValid(self, path):
        """ @rtype: tuple of (bool, str) """
        path = path.strip()
        path_exists = os.path.exists(path)
        is_file = os.path.isfile(path)
        is_directory = os.path.isdir(path)
        is_executable = os.access(path, os.X_OK)
        is_absolute = os.path.isabs(path)

        valid = True
        message = ""

        if path == "":
            if self._model.pathIsRequired():
                valid = False
                message = PathChooser.REQUIRED_FIELD_MSG
        elif not path_exists:
            if self._model.pathMustExist():
                valid = False
                message = PathChooser.PATH_DOES_NOT_EXIST_MSG
            #todo: check if new (non-existing) file has directory or file format?
        elif path_exists:
            if self._model.pathMustBeExecutable() and is_file and not is_executable:
                valid = False
                message = PathChooser.FILE_IS_NOT_EXECUTABLE_MSG
            elif self._model.pathMustBeADirectory() and not is_directory:
                valid = False
                message = PathChooser.PATH_IS_NOT_A_DIRECTORY_MSG
            elif self._model.pathMustBeAbsolute() and not is_absolute:
                valid = False
                message = PathChooser.PATH_IS_NOT_ABSOLUTE_MSG
            elif self._model.pathMustBeAFile() and not is_file:
                valid = False
                message = PathChooser.PATH_IS_NOT_A_FILE_MSG

        return valid, message


    def validatePath(self):
        """Called whenever the path is modified"""
        palette = self._path_line.palette()

        valid, message = self.isPathValid(self.getPath())

        validity_type = ValidationSupport.WARNING

        if not valid:
            color = ValidationSupport.ERROR_COLOR
        else:
            color = self.valid_color

        self._validation_support.setValidationMessage(message, validity_type)
        self._path_line.setToolTip(message)
        palette.setColor(self._path_line.backgroundRole(), color)

        self._path_line.setPalette(palette)


    def getPath(self):
        """Returns the path"""
        return os.path.expanduser(str(self._path_line.text()).strip())

    def pathExists(self):
        """Returns True if the entered path exists"""
        return os.path.exists(self.getPath())

    def selectPath(self):
        """Pops up the 'select a file/directory' dialog"""
        # todo: This probably needs some reworking to work properly with different scenarios... (file + dir)
        self._editing = True
        current_directory = self.getPath()

        #if not os.path.exists(currentDirectory):
        #    currentDirectory = "~"

        if self._model.pathMustBeAFile():
            current_directory = QFileDialog.getOpenFileName(self, "Select a file path", current_directory)
        else:
            current_directory = QFileDialog.getExistingDirectory(self, "Select a directory", current_directory)

        if not current_directory == "":
            if not self._model.pathMustBeAbsolute():
                cwd = os.getcwd()
                match = re.match(cwd + "/(.*)", current_directory)
                if match:
                    current_directory = match.group(1)

            self._path_line.setText(current_directory)
            self._model.setPath(self.getPath())

        self._editing = False


    def contentsChanged(self):
        """Called whenever the path is changed."""
        path_is_valid, message = self.isPathValid(self.getPath())

        if not self._editing and path_is_valid:
            self._model.setPath(self.getPath())


    def getPathFromModel(self):
        """Retrieves data from the model and inserts it into the edit line"""
        self._editing = True

        path = self._model.getPath()
        if path is None:
            path = ""

        self._path_line.setText("%s" % path)
        self._editing = False


    def getValidationSupport(self):
        return self._validation_support

    def isValid(self):
        return self._validation_support.isValid()




