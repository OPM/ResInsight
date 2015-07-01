#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'string_box.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert_gui.widgets.helped_widget import HelpedWidget


class OptionWidget(HelpedWidget):
    """Shows a tab widget. Validation status is against the current selected tab/widget."""

    def __init__(self, path_label="Option", help_link=""):
        HelpedWidget.__init__(self, path_label, help_link)

        self.tab_widget = QtGui.QTabWidget()
        self.tab_widget.currentChanged.connect(self.validateTabs)
        self.addWidget(self.tab_widget)
        self.__widgets = {}
        """ :type: dict of (int, HelpedWidget) """


    def validateTabs(self):
        valid = self.isValid()

        if not valid:
            self.validationChanged.emit(False)
        else:
            self.validationChanged.emit(True)

    def addHelpedWidget(self, label, helped_widget):
        """
        @type label: str
        @type helped_widget: HelpedWidget
        """

        index = self.tab_widget.addTab(helped_widget, label)
        self.__widgets[index] = helped_widget
        helped_widget.validationChanged.connect(self.validateTabs)

    def isValid(self):
        current_widget = self.getCurrentWidget()

        if current_widget is not None:
            return current_widget.isValid()
        return False

    def getCurrentWidget(self):
        index = self.tab_widget.currentIndex()
        if index in self.__widgets:
            return self.__widgets[index]
        return None



