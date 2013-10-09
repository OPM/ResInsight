#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'active_label.py' is part of ERT - Ensemble based Reservoir Tool.
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
from PyQt4.QtGui import QLabel, QFont
from ert_gui.models.mixins import TextModelMixin
from ert_gui.widgets.helped_widget import HelpedWidget


class ActiveLabel(HelpedWidget):
    """Label shows a string. The model must be a TextModelMixin"""

    def __init__(self, model, label="", help_link="", default_string=""):
        HelpedWidget.__init__(self, label, help_link)

        assert isinstance(model, TextModelMixin)
        self.model = model
        self.model.observable().attach(TextModelMixin.INITIALIZED_EVENT, self.modelChanged)
        self.model.observable().attach(TextModelMixin.TEXT_VALUE_CHANGED_EVENT, self.modelChanged)

        self.active_label = QLabel()
        self.addWidget(self.active_label)

        font = self.active_label.font()
        font.setWeight(QFont.Bold)
        self.active_label.setFont(font)

        self.active_label.setText(default_string)


    def modelChanged(self):
        """Retrieves data from the model and inserts it into the edit line"""
        self_get_from_model = self.model.getText()
        if self_get_from_model is None:
            self_get_from_model = ""

        self.active_label.setText(str(self_get_from_model))
