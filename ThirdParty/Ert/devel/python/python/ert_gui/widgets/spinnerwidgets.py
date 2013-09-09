#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'spinnerwidgets.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from helpedwidget import HelpedWidget

class IntegerSpinner(HelpedWidget):
    """A spinner widget for integers. The data structure expected and sent to the getter and setter is an integer."""
    def __init__(self, parent=None, spinnerLabel="Number", help="", min=0, max=10):
        """Construct a spinner widget for integers"""
        HelpedWidget.__init__(self, parent, spinnerLabel, help)

        self.spinner = QtGui.QSpinBox(self)
        self.spinner.setMinimum(min)
        self.spinner.setMaximum(max)
        #self.connect(self.pathLine, QtCore.SIGNAL('textChanged(QString)'), self.validatePath)
        self.addWidget(self.spinner)

        self.infoLabel = QtGui.QLabel()
        self.infoLabel.setHidden(True)
        self.addWidget(self.infoLabel)

        self.addStretch()
        self.addHelpButton()

        #self.connect(self.spinner, QtCore.SIGNAL('valueChanged(int)'), self.updateContent)
        self.connect(self.spinner, QtCore.SIGNAL('editingFinished()'), self.contentsChanged)

    def contentsChanged(self):
        """Called whenever the contents of the spinner changes."""
        self.updateContent(self.spinner.value())

    def fetchContent(self):
        """Retrieves data from the model and inserts it into the spinner"""
        self.spinner.setValue(self.getFromModel())

    def setInfo(self, info):
        self.infoLabel.setText(info)
        self.infoLabel.setHidden(False)

class DoubleSpinner(HelpedWidget):
    """A spinner widget for doubles. The data structure expected and sent to the getter and setter is a double."""
    def __init__(self, parent=None, spinnerLabel="Double Number", help="", min=0.0, max=1.0, decimals=2):
        """Construct a spinner widget for doubles"""
        HelpedWidget.__init__(self, parent, spinnerLabel, help)

        self.spinner = QtGui.QDoubleSpinBox(self)
        self.spinner.setMinimum(min)
        self.spinner.setMaximum(max)
        self.spinner.setDecimals(decimals)
        self.spinner.setSingleStep(0.01)

        self.addWidget(self.spinner)

        self.addStretch()
        self.addHelpButton()

        #self.connect(self.spinner, QtCore.SIGNAL('valueChanged(int)'), self.updateContent)
        self.connect(self.spinner, QtCore.SIGNAL('editingFinished()'), self.contentsChanged)

    def contentsChanged(self):
        """Called whenever the contents of the spinner changes."""
        self.updateContent(self.spinner.value())


    def fetchContent(self):
        """Retrieves data from the model and inserts it into the spinner"""
        self.spinner.setValue(self.getFromModel())
