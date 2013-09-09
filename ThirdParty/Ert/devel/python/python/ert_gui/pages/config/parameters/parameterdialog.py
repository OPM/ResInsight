#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'parameterdialog.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert_gui.widgets.validateddialog import ValidatedDialog

class ParameterDialog(ValidatedDialog):
    """A dialog for creating parameters based on type and name. Performs validation of name."""

    def __init__(self, parent, types, uniqueNames):
        """Creates a new dialog that validates uniqueness against the provided list"""
        ValidatedDialog.__init__(self, parent, 'Create new parameter', "Select type and enter name of parameter:", uniqueNames)

        self.paramCombo = QtGui.QComboBox(self)

        keys = types.keys()
        keys.sort()
        for key in keys:
            self.paramCombo.addItem(types[key], key.name)

        self.layout.insertRow(2, "Type:", self.paramCombo)


    def getTypeName(self):
        """Return the type selected by the user"""
        return str(self.paramCombo.currentText()).strip()

