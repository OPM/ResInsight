#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'checkbox.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from helpedwidget import *

class CheckBox(HelpedWidget):
    """A checbox widget for booleans. The data structure expected and sent to the getter and setter is a boolean."""

    def __init__(self, parent=None, checkLabel="Do this", help="", altLabel="", defaultCheckState=True):
        """Construct a checkbox widget for booleans"""
        HelpedWidget.__init__(self, parent, checkLabel, help)

        if altLabel:
            self.check = QtGui.QCheckBox(altLabel, self)
        else:
            self.check = QtGui.QCheckBox(checkLabel, self)

        self.check.setTristate(False)

        if defaultCheckState:
            self.check.setCheckState(QtCore.Qt.Checked)
        else:
            self.check.setCheckState(QtCore.Qt.Unchecked)


        self.addWidget(self.check)

        self.addStretch()
        self.addHelpButton()

        #self.connect(self.spinner, QtCore.SIGNAL('valueChanged(int)'), self.updateContent)
        self.connect(self.check, QtCore.SIGNAL('stateChanged(int)'), self.contentsChanged)


    def contentsChanged(self):
        """Called whenever the contents of the checbox changes."""
        if self.check.checkState() == QtCore.Qt.Checked:
            self.updateContent(True)
        else:
            self.updateContent(False)


    def fetchContent(self):
        """Retrieves data from the model and inserts it into the checkbox"""
        if self.getFromModel():
            self.check.setCheckState(QtCore.Qt.Checked)
        else:
            self.check.setCheckState(QtCore.Qt.Unchecked)

