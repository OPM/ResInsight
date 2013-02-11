#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'configpanel.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from util import createSeparator

class ConfigPanel(QtGui.QTabWidget):
    """Convenience class for a tabbed configuration panel"""

    def __init__(self, parent=None):
        """Creates a config panel widget"""
        QtGui.QTabWidget.__init__(self, parent)
        self.layoutQueue = []


    def startPage(self, name):
        """Starts a new page of the configuration panel"""
        self.pageName = name
        self.contentPage = QtGui.QWidget()
        self.formLayout = QtGui.QFormLayout()
        self.formLayout.setLabelAlignment(QtCore.Qt.AlignRight)


    def addRow(self, row, label=None):
        """
        Add a new row on a configuration page. Returns the row widget.
        If the row does not have a getLabel() function the row spans both columns.
        """
        if hasattr(row, "getLabel") and not row.getLabel() == "":            
            self.formLayout.addRow(row.getLabel(), row)
        else:
            if label is None:
                self.formLayout.addRow(row)
            else:
                self.formLayout.addRow(label, row)
            
        return row

    def endPage(self):
        """Indicate the end of a complete configuration page."""
        self.contentPage.setLayout(self.formLayout)
        self.addTab(self.contentPage, self.pageName)

        self.contentPage = None
        self.formLayout = None
        self.pageName = None

    def startGroup(self, groupTitle):
        """Start a titled sub group on the page."""
        self.groupBox = QtGui.QGroupBox(groupTitle)
        self.layoutQueue.append(self.formLayout)
        self.formLayout = QtGui.QFormLayout()
        self.formLayout.setLabelAlignment(QtCore.Qt.AlignRight)

    def endGroup(self):
        """Finish the titled sub group"""
        self.groupBox.setLayout(self.formLayout)

        self.formLayout = self.layoutQueue.pop()
        self.formLayout.addRow(self.groupBox)
        self.groupBox = None

    def addSeparator(self):
        """Adds a separator line to the panel."""

        self.formLayout.addRow(createSeparator())
