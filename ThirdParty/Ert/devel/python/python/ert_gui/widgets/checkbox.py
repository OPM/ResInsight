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


from PyQt4 import QtCore
from PyQt4.QtCore import Qt
from PyQt4.QtGui import QCheckBox, QHBoxLayout

from ert_gui.models.mixins import BooleanModelMixin
from ert_gui.widgets.helped_widget import HelpedWidget


class CheckBox(HelpedWidget):
    """A checbox widget for booleans. The data structure expected and sent to the getter and setter is a boolean."""

    def __init__(self, model, label="Checkbox", help_link="", show_label=True, alt_label=None, default_check_state=True):
        """Construct a checkbox widget for booleans"""
        HelpedWidget.__init__(self, label, help_link)

        if show_label:
            if alt_label is not None:
                self.check = QCheckBox(alt_label, self)
            else:
                self.check = QCheckBox(label,self)
        else:
            self.check = QCheckBox(self)
            
        self.check.setTristate(False)
        self.check.setChecked(default_check_state)
        self.connect(self.check, QtCore.SIGNAL('stateChanged(int)'), self.contentsChanged)

        if not show_label:
            layout = QHBoxLayout()
            layout.addWidget(self.check)
            layout.setAlignment(Qt.AlignCenter)
            layout.setContentsMargins(0, 0, 0, 0)
            self.addLayout(layout)
        else:
            self.addWidget(self.check)

        assert isinstance(model, BooleanModelMixin)
        self.model = model
        self.model.observable().attach(BooleanModelMixin.BOOLEAN_VALUE_CHANGED_EVENT, self.modelChanged)
        self.modelChanged()


    def contentsChanged(self):
        """Called whenever the contents of the checbox changes."""
        self.model.setState(self.check.isChecked())


    def modelChanged(self):
        """Retrives data from the model and sets the state of the checkbox."""
        self.check.setChecked(self.model.isTrue())

