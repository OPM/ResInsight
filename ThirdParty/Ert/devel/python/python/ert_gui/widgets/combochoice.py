#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'combochoice.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import sys
from PyQt4 import QtGui, QtCore
from helpedwidget import *

class ComboChoice(HelpedWidget):
    """
    A combo box widget for choices. The data structure expected and sent to the getter and setter is a string
    that is equal to one of the available ones.
    """
    def __init__(self, parent=None, choiceList=None, comboLabel="Choice", help=""):
        """Construct a ComboChoice widget"""
        HelpedWidget.__init__(self, parent, comboLabel, help)

        self.combo = QtGui.QComboBox(self)

        if choiceList is None:
            choiceList = ["No choices"]

        for choice in choiceList:
            self.combo.addItem(str(choice))

        self.addWidget(self.combo)
        self.addStretch()
        self.addHelpButton()

        self.connect(self.combo, QtCore.SIGNAL('currentIndexChanged(QString)'), self.updateContent)


    def fetchContent(self):
        """Retrieves data from the model and updates the combo box."""
        newValue = self.getFromModel()

        indexSet = False
        for i in range(self.combo.count()):
            if str(self.combo.itemText(i)).lower() == str(newValue).lower():
                self.combo.setCurrentIndex(i)
                indexSet = True
                break

        if not indexSet:
            self.combo.setCurrentIndex(0)
            sys.stderr.write("AssertionError: ComboBox can not be set to: " + str(newValue) + "\n")
            #raise AssertionError("ComboBox can not be set to: " + str(newValue))



    def updateList(self, choiceList):
        """Replace the list of choices with the specified items"""
        self.disconnect(self.combo, QtCore.SIGNAL('currentIndexChanged(QString)'), self.updateContent)

        self.combo.clear()
        for choice in choiceList:
            self.combo.addItem(choice)

        self.connect(self.combo, QtCore.SIGNAL('currentIndexChanged(QString)'), self.updateContent)
