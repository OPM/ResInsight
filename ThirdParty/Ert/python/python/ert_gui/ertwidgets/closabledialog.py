#  Copyright (C) 2016  Statoil ASA, Norway.
#
#  The file 'closabledialog.py' is part of ERT - Ensemble based Reservoir Tool.
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

from PyQt4.QtCore import Qt
from PyQt4.QtGui import  QDialog, QVBoxLayout, QLayout, QPushButton, QHBoxLayout


class ClosableDialog(QDialog):

    def __init__(self, title, widget, parent=None):
        QDialog.__init__(self, parent)

        self.setWindowTitle(title)
        self.setModal(True)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowContextHelpButtonHint)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowCloseButtonHint)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowCancelButtonHint)


        layout = QVBoxLayout()
        layout.setSizeConstraint(QLayout.SetFixedSize) # not resizable!!!
        layout.addWidget(widget)

        self.__button_layout = QHBoxLayout()
        self.close_button = QPushButton("Close")
        self.close_button.setObjectName("CLOSE")
        self.close_button.clicked.connect(self.accept)
        self.__button_layout.addStretch()
        self.__button_layout.addWidget(self.close_button)

        layout.addStretch()
        layout.addLayout(self.__button_layout)

        self.setLayout(layout)


    def disableCloseButton(self):
        self.close_button.setEnabled(False)

    def enableCloseButton(self):
        self.close_button.setEnabled(True)

    def keyPressEvent(self, q_key_event):
        if not self.close_button.isEnabled() and q_key_event.key() == Qt.Key_Escape:
            pass
        else:
            QDialog.keyPressEvent(self, q_key_event)

    def addButton(self, caption, listner):
        button = QPushButton(caption)
        button.setObjectName(str(caption).capitalize())
        self.__button_layout.insertWidget(1,button)
        button.clicked.connect(listner)

    def toggleButton(self, caption, enabled):
        button = self.findChild(QPushButton,str(caption).capitalize())
        if button is not None:
            button.setEnabled(enabled)


