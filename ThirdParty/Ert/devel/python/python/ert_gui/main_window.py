from PyQt4.QtCore import QSettings, Qt
from PyQt4.QtGui import QMainWindow, qApp, QWidget, QVBoxLayout, QDockWidget, QAction, QToolButton
from ert_gui.about_dialog import AboutDialog


class GertMainWindow(QMainWindow):
    def __init__(self):
        QMainWindow.__init__(self)

        self.tools = {}

        self.resize(300, 700)
        self.setWindowTitle('ERT')

        self.__main_widget = None

        self.central_widget = QWidget()
        self.central_layout = QVBoxLayout()
        self.central_widget.setLayout(self.central_layout)

        self.setCentralWidget(self.central_widget)

        self.toolbar = self.addToolBar("Tools")
        self.toolbar.setObjectName("Toolbar")
        self.toolbar.setToolButtonStyle(Qt.ToolButtonTextUnderIcon)

        self.setCorner(Qt.TopLeftCorner, Qt.LeftDockWidgetArea)
        self.setCorner(Qt.BottomLeftCorner, Qt.BottomDockWidgetArea)

        self.setCorner(Qt.TopRightCorner, Qt.RightDockWidgetArea)
        self.setCorner(Qt.BottomRightCorner, Qt.BottomDockWidgetArea)
        self.__view_menu = None
        self.__help_menu = None

        self.__createMenu()
        self.__fetchSettings()

    def addDock(self, name, widget, area=Qt.RightDockWidgetArea, allowed_areas=Qt.AllDockWidgetAreas):
        dock_widget = QDockWidget(name)
        dock_widget.setObjectName("%sDock" % name)
        dock_widget.setWidget(widget)
        dock_widget.setAllowedAreas(allowed_areas)

        self.addDockWidget(area, dock_widget)

        self.__view_menu.addAction(dock_widget.toggleViewAction())
        return dock_widget

    def addTool(self, tool):
        tool.setParent(self)
        self.tools[tool.getName()] = tool
        self.toolbar.addAction(tool.getAction())

        if tool.isPopupMenu():
            tool_button = self.toolbar.widgetForAction(tool.getAction())
            tool_button.setPopupMode(QToolButton.InstantPopup)


    def __createMenu(self):
        file_menu = self.menuBar().addMenu("&File")
        file_menu.addAction("Close", self.__quit)
        self.__view_menu = self.menuBar().addMenu("&View")
        self.__help_menu = self.menuBar().addMenu("&Help")
        """:type: QMenu"""

        """ @rtype: list of QAction """
        advanced_toggle_action = QAction("Show Advanced Options", self)
        advanced_toggle_action.setObjectName("AdvancedSimulationOptions")
        advanced_toggle_action.setCheckable(True)
        advanced_toggle_action.setChecked(False)
        advanced_toggle_action.toggled.connect(self.toggleAdvancedMode)

        self.__view_menu.addAction(advanced_toggle_action)

        """ @rtype: list of QAction """
        show_about = self.__help_menu.addAction("About")
        show_about.setMenuRole(QAction.ApplicationSpecificRole)
        show_about.triggered.connect(self.__showAboutMessage)


    def __quit(self):
        self.__saveSettings()
        qApp.quit()


    def __saveSettings(self):
        settings = QSettings("Statoil", "Ert-Gui")
        settings.setValue("geometry", self.saveGeometry())
        settings.setValue("windowState", self.saveState())


    def closeEvent(self, event):
        #Use QT settings saving mechanism
        #settings stored in ~/.config/Statoil/ErtGui.conf
        self.__saveSettings()
        QMainWindow.closeEvent(self, event)


    def __fetchSettings(self):
        settings = QSettings("Statoil", "Ert-Gui")
        self.restoreGeometry(settings.value("geometry").toByteArray())
        self.restoreState(settings.value("windowState").toByteArray())

    def toggleAdvancedMode(self, advanced_mode):
        if hasattr(self.__main_widget, "toggleAdvancedMode"):
            self.__main_widget.toggleAdvancedMode(advanced_mode)

        for tool in self.tools.values():
            if hasattr(tool, "toggleAdvancedMode"):
                tool.toggleAdvancedMode(advanced_mode)


    def setWidget(self, widget):
        self.__main_widget = widget
        actions = widget.getActions()
        for action in actions:
            self.__view_menu.addAction(action)

        self.central_layout.addWidget(widget)

    def __showAboutMessage(self):
        diag = AboutDialog(self)
        diag.show()
        pass




