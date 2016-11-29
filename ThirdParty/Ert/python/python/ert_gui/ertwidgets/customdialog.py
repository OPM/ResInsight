#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'customdialog.py' is part of ERT - Ensemble based Reservoir Tool.
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
from PyQt4.QtGui import QDialog, QColor, QFormLayout, QLabel, QWidget, QDialogButtonBox, QLayout


class CustomDialog(QDialog):
    INVALID_COLOR = QColor(255, 235, 235)

    def __init__(self, title="Title", description="Description", parent=None):
        QDialog.__init__(self, parent)

        self._option_list = []
        """ :type: list of QWidget """

        self.setModal(True)
        self.setWindowTitle(title)

        self.layout = QFormLayout()
        self.layout.setFieldGrowthPolicy(QFormLayout.ExpandingFieldsGrow)
        self.layout.setSizeConstraint(QLayout.SetFixedSize)

        label = QLabel(description)
        label.setAlignment(Qt.AlignHCenter)

        self.layout.addRow(self.createSpace(5))
        self.layout.addRow(label)
        self.layout.addRow(self.createSpace(10))

        self.ok_button = None

        self.setLayout(self.layout)

    def notValid(self, msg):
        """Called when the name is not valid."""
        self.ok_button.setEnabled(False)

    def valid(self):
        """Called when the name is valid."""
        self.ok_button.setEnabled(True)

    def optionValidationChanged(self):
        valid = True
        for option in self._option_list:
            if hasattr(option, "isValid"):
                if not option.isValid():
                    valid = False
                    self.notValid("One or more options are incorrectly set!")

        if valid:
            self.valid()

    def showAndTell(self):
        """
        Shows the dialog modally and returns the true or false (accept/reject)
        @rtype: bool
        """
        self.optionValidationChanged()
        return self.exec_()

    def createSpace(self, size=5):
        """Creates a widget that can be used as spacing on  a panel."""
        qw = QWidget()
        qw.setMinimumSize(QSize(size, size))

        return qw

    def addSpace(self, size=10):
        """ Add some vertical spacing """
        space_widget = self.createSpace(size)
        self.layout.addRow("", space_widget)

    def addLabeledOption(self, label, option_widget):
        """
        @type option_widget: QWidget
        """
        self._option_list.append(option_widget)

        if hasattr(option_widget, "validationChanged"):
            option_widget.validationChanged.connect(self.optionValidationChanged)

        if hasattr(option_widget, "getValidationSupport"):
            validation_support = option_widget.getValidationSupport()
            validation_support.validationChanged.connect(self.optionValidationChanged)

        self.layout.addRow("%s:" % label, option_widget)

    def addWidget(self, widget, label=""):
        if not label.endswith(":"):
            label = "%s:" % label
        self.layout.addRow(label, widget)

    def addButtons(self):
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel, Qt.Horizontal, self)
        self.ok_button = buttons.button(QDialogButtonBox.Ok)
        self.ok_button.setEnabled(False)

        self.layout.addRow(self.createSpace(10))
        self.layout.addRow(buttons)

        self.connect(buttons, SIGNAL('accepted()'), self.accept)
        self.connect(buttons, SIGNAL('rejected()'), self.reject)
