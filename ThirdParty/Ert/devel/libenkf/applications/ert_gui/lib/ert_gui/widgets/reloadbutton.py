#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'reloadbutton.py' is part of ERT - Ensemble based Reservoir Tool. 
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

class ReloadButton(HelpedWidget):
    """Presents a reload button. """

    def __init__(self, parent=None, label="", help="", button_text=""):
        """Construct a StringBox widget"""
        HelpedWidget.__init__(self, parent, label, help)

        self.button = QtGui.QToolButton(self)
        self.button.setText(button_text)
        self.addWidget(self.button)

        self.connect(self.button, QtCore.SIGNAL('clicked()'), self.fetchContent)

        self.addStretch()
        self.addHelpButton()


    def fetchContent(self):
        """Retrieves data from the model"""
        data = self.getFromModel()
