#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'application.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from PyQt4.QtGui import QDockWidget
from PyQt4.QtCore import Qt, QSettings

class Application(QtGui.QMainWindow):
    """An application (window widget) with a list of "tasks" on the left side and a panel on the right side"""

    def __init__(self):
        """Constructor"""
        QtGui.QMainWindow.__init__(self)

        self.resize(900, 700)
        self.setWindowTitle('ERT GUI')

        centralWidget = QtGui.QWidget()
        widgetLayout = QtGui.QVBoxLayout()

        self.contentsWidget = QtGui.QListWidget()
        self.contentsWidget.setViewMode(QtGui.QListView.IconMode)
        self.contentsWidget.setIconSize(QtCore.QSize(96, 96))
        self.contentsWidget.setMovement(QtGui.QListView.Static)
        self.contentsWidget.setMaximumWidth(128)
        self.contentsWidget.setMinimumWidth(128)
        self.contentsWidget.setSpacing(12)

        dock = self._createDock()

        self.addDockWidget(Qt.LeftDockWidgetArea, dock)

        self.pagesWidget = QtGui.QStackedWidget()


        horizontalLayout = QtGui.QHBoxLayout()
        horizontalLayout.addWidget(self.pagesWidget, 1)
        widgetLayout.addLayout(horizontalLayout)

        self._createMenu(dock)

        centralWidget.setLayout(widgetLayout)
        self.setCentralWidget(centralWidget)

        self.save_function = None

        self._fetchSettings()

    def setSaveFunction(self, save_function):
        """Set the function to be called when the save menu choice is selected."""
        self.save_function = save_function

    def _save(self):
        if not self.save_function is None:
            self.save_function()

    def _createDock(self):
        dock = QDockWidget("")
        dock.setObjectName("ERTGUI Workflow")
        dock.setWidget(self.contentsWidget)
        dock.setFeatures(QDockWidget.DockWidgetClosable)
        dock.setAllowedAreas(Qt.LeftDockWidgetArea)
        return dock

    def _createMenu(self, dock):
        file_menu = self.menuBar().addMenu("&File")
        file_menu.addAction("Save configuration", self._save)
        file_menu.addAction("Close", self._quit)
        
        self.view_menu = self.menuBar().addMenu("&View")
        self.view_menu.addAction(dock.toggleViewAction())
        self.view_menu.addSeparator()

    def addPage(self, name, icon, page):
        """Add another page to the application"""
        button = QtGui.QListWidgetItem(self.contentsWidget)
        button.setIcon(icon)
        button.setText(name)
        button.setTextAlignment(QtCore.Qt.AlignHCenter)
        button.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled)

        def switchPage():
            self.contentsWidget.setCurrentRow(self.contentsWidget.row(button))

        self.view_menu.addAction(name, switchPage)

        self.pagesWidget.addWidget(page)
        self.connect(self.contentsWidget, QtCore.SIGNAL('currentItemChanged(QListWidgetItem *, QListWidgetItem *)'), self._changePage)

        self.contentsWidget.setCurrentRow(0)
        

    def _changePage(self, current, previous):
        """Switch page. Connected to the: currentItemChanged() signal of the list widget on the left side"""
        if current is None:
            current = previous

        self.pagesWidget.setCurrentIndex(self.contentsWidget.row(current))

    def _quit(self):
        self._saveSettings()
        QtGui.qApp.quit()

    def _saveSettings(self):
        settings = QSettings("Statoil", "ErtGui")
        settings.setValue("geometry", self.saveGeometry())
        settings.setValue("windowState", self.saveState())

    def closeEvent(self, event):
        #Use QT settings saving mechanism
        #settings stored in ~/.config/Statoil/ErtGui.conf
        self._saveSettings()
        QtGui.QMainWindow.closeEvent(self, event)

    def _fetchSettings(self):
        settings = QSettings("Statoil", "ErtGui")
        self.restoreGeometry(settings.value("geometry").toByteArray())
        self.restoreState(settings.value("windowState").toByteArray())
