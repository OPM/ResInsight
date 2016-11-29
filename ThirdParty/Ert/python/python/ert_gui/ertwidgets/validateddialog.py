#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'validateddialog.py' is part of ERT - Ensemble based Reservoir Tool.
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
from PyQt4.QtCore import Qt, QSize, SIGNAL
from PyQt4.QtGui import QDialog, QColor, QFormLayout, QLabel, QWidget, QDialogButtonBox, QLineEdit, QComboBox, QLayout


class ValidatedDialog(QDialog):
    """
    A dialog for creating a validated new value. Performs validation of name against a provided.
    Can be used to select from the list or for creating a new value that is not on the list.

    """

    INVALID_COLOR = QColor(255, 235, 235)

    def __init__(self, title="Title", description="Description", unique_names=None, choose_from_list=False):
        QDialog.__init__(self)
        self.setModal(True)
        self.setWindowTitle(title)
        # self.setMinimumWidth(250)
        # self.setMinimumHeight(150)

        if unique_names is None:
            unique_names = []

        self.unique_names = unique_names
        self.choose_from_list = choose_from_list

        self.layout = QFormLayout()
        self.layout.setSizeConstraint(QLayout.SetFixedSize)

        label = QLabel(description)
        label.setAlignment(Qt.AlignHCenter)

        self.layout.addRow(self.createSpace(5))
        self.layout.addRow(label)
        self.layout.addRow(self.createSpace(10))

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel, Qt.Horizontal, self)
        self.ok_button = buttons.button(QDialogButtonBox.Ok)
        self.ok_button.setEnabled(False)

        if choose_from_list:
            self.param_name_combo = QComboBox()
            self.connect(self.param_name_combo, SIGNAL('currentIndexChanged(QString)'), self.validateChoice)
            for item in unique_names:
                self.param_name_combo.addItem(item)
            self.layout.addRow("Job:", self.param_name_combo)
        else:
            self.param_name = QLineEdit(self)
            self.param_name.setFocus()
            self.connect(self.param_name, SIGNAL('textChanged(QString)'), self.validateName)
            self.validColor = self.param_name.palette().color(self.param_name.backgroundRole())

            self.layout.addRow("Name:", self.param_name)

        self.layout.addRow(self.createSpace(10))

        self.layout.addRow(buttons)

        self.connect(buttons, SIGNAL('accepted()'), self.accept)
        self.connect(buttons, SIGNAL('rejected()'), self.reject)

        self.setLayout(self.layout)

    def notValid(self, msg):
        """Called when the name is not valid."""
        self.ok_button.setEnabled(False)
        palette = self.param_name.palette()
        palette.setColor(self.param_name.backgroundRole(), self.INVALID_COLOR)
        self.param_name.setToolTip(msg)
        self.param_name.setPalette(palette)

    def valid(self):
        """Called when the name is valid."""
        self.ok_button.setEnabled(True)
        palette = self.param_name.palette()
        palette.setColor(self.param_name.backgroundRole(), self.validColor)
        self.param_name.setToolTip("")
        self.param_name.setPalette(palette)

    def validateName(self, value):
        """Called to perform validation of a name. For specific needs override this function and call valid() and notValid(msg)."""
        value = str(value)

        if value == "":
            self.notValid("Can not be empty!")
        elif not value.find(" ") == -1:
            self.notValid("No spaces allowed!")
        elif value in self.unique_names:
            self.notValid("Name must be unique!")
        else:
            self.valid()

    def validateChoice(self, choice):
        """Only called when using selection mode."""
        self.ok_button.setEnabled(not choice == "")

    def getName(self):
        """Return the new name chosen by the user"""
        if self.choose_from_list:
            return str(self.param_name_combo.currentText())
        else:
            return str(self.param_name.text())

    def showAndTell(self):
        """Shows the dialog and returns the result"""
        if self.exec_():
            return str(self.getName()).strip()

        return ""

    def createSpace(self, size=5):
        """Creates a widget that can be used as spacing on  a panel."""
        qw = QWidget()
        qw.setMinimumSize(QSize(size, size))

        return qw
