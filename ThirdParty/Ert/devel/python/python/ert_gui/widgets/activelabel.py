#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'activelabel.py' is part of ERT - Ensemble based Reservoir Tool. 
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

class ActiveLabel(HelpedWidget):
    """Label shows a string. The data structure expected from the getter is a string."""

    def __init__(self, parent=None, label="", help="", default_string=""):
        """Construct a StringBox widget"""
        HelpedWidget.__init__(self, parent, label, help)

        self.active_label = QtGui.QLabel()
        self.addWidget(self.active_label)

        font = self.active_label.font()
        font.setWeight(QtGui.QFont.Bold)
        self.active_label.setFont(font)

        #self.addHelpButton()

        self.active_label.setText(default_string)


    def fetchContent(self):
        """Retrieves data from the model and inserts it into the edit line"""
        self_get_from_model = self.getFromModel()
        if self_get_from_model is None:
            self_get_from_model = ""

        self.active_label.setText("%s" % self_get_from_model)
