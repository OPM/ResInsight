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

    def __init__(self, model, path_label="Path", help_link=""):
        HelpedWidget.__init__(self, path_label, help_link)

        self.__editing = True
        self.__valid = True

        self.path_line = QLineEdit()
        self.path_line.setMinimumWidth(250)

        self.addWidget(self.path_line)

        dialog_button = QToolButton(self)
        dialog_button.setIcon(resourceIcon("ide/small/folder"))
        dialog_button.setIconSize(QSize(16, 16))
        self.connect(dialog_button, SIGNAL('clicked()'), self.selectPath)
        self.addWidget(dialog_button)

        self.valid_color = self.path_line.palette().color(self.path_line.backgroundRole())

        self.path_line.setText(os.getcwd())
        self.__editing = False

        assert isinstance(model, PathModelMixin)
        self.model = model
        model.observable().attach(PathModelMixin.PATH_CHANGED_EVENT, self.getPathFromModel)

        self.connect(self.path_line, SIGNAL('editingFinished()'), self.validatePath)
        self.connect(self.path_line, SIGNAL('editingFinished()'), self.contentsChanged)
        self.connect(self.path_line, SIGNAL('textChanged(QString)'), self.validatePath)

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
            if self.model.pathIsRequired():
                valid = False
                message = PathChooser.REQUIRED_FIELD_MSG
        elif not path_exists:
            if self.model.pathMustExist():
                valid = False
                message = PathChooser.PATH_DOES_NOT_EXIST_MSG
            #todo: check if new (non-existing) file has directory or file format?
        elif path_exists:
            if self.model.pathMustBeExecutable() and is_file and not is_executable:
                valid = False
                message = PathChooser.FILE_IS_NOT_EXECUTABLE_MSG
            elif self.model.pathMustBeADirectory() and not is_directory:
                valid = False
                message = PathChooser.PATH_IS_NOT_A_DIRECTORY_MSG
            elif self.model.pathMustBeAbsolute() and not is_absolute:
                valid = False
                message = PathChooser.PATH_IS_NOT_ABSOLUTE_MSG
            elif self.model.pathMustBeAFile() and not is_file:
                valid = False
                message = PathChooser.PATH_IS_NOT_A_FILE_MSG

        return valid, message


    def validatePath(self):
        """Called whenever the path is modified"""
        palette = self.path_line.palette()

        valid, message = self.isPathValid(self.getPath())

        validity_type = self.WARNING

        if not valid:
            color = self.ERROR_COLOR
        else:
            color = self.valid_color

        self.__valid = valid

        self.setValidationMessage(message, validity_type)
        self.path_line.setToolTip(message)
        palette.setColor(self.path_line.backgroundRole(), color)

        self.path_line.setPalette(palette)


    def getPath(self):
        """Returns the path"""
        return os.path.expanduser(str(self.path_line.text()).strip())

    def pathExists(self):
        """Returns True if the entered path exists"""
        return os.path.exists(self.getPath())
        
    def isValid(self):
        """Returns the validation value"""
        return self.__valid


    def selectPath(self):
        """Pops up the 'select a file/directory' dialog"""
        # todo: This probably needs some reworking to work properly with different scenarios... (file + dir)
        self.__editing = True
        current_directory = self.getPath()

        #if not os.path.exists(currentDirectory):
        #    currentDirectory = "~"

        if self.model.pathMustBeAFile():
            current_directory = QFileDialog.getOpenFileName(self, "Select a file path", current_directory)
        else:
            current_directory = QFileDialog.getExistingDirectory(self, "Select a directory", current_directory)

        if not current_directory == "":
            if not self.model.pathMustBeAbsolute():
                cwd = os.getcwd()
                match = re.match(cwd + "/(.*)", current_directory)
                if match:
                    current_directory = match.group(1)

            self.path_line.setText(current_directory)
            self.model.setPath(self.getPath())

        self.__editing = False


    def contentsChanged(self):
        """Called whenever the path is changed."""
        path_is_valid, message = self.isPathValid(self.getPath())

        if not self.__editing and path_is_valid:
            self.model.setPath(self.getPath())


    def getPathFromModel(self):
        """Retrieves data from the model and inserts it into the edit line"""
        self.__editing = True

        path = self.model.getPath()
        if path is None:
            path = ""

        self.path_line.setText("%s" % path)
        self.__editing = False


    def cleanup(self):
        self.model.observable().detach(PathModelMixin.PATH_CHANGED_EVENT, self.getPathFromModel)

