from PyQt4.QtCore import pyqtSignal
from PyQt4.QtGui import QMainWindow

from ert_gui.tools.ide.configuration_panel import ConfigurationPanel


class IdeWindow(QMainWindow):
    reloadTriggered = pyqtSignal(str)

    def __init__(self, path, parent, help_tool):
        QMainWindow.__init__(self, parent)

        self.resize(900, 900)

        self.__position = None
        self.__geometry = None

        self.__configuration_panel = ConfigurationPanel(path, help_tool)
        self.__configuration_panel.reloadApplication.connect(self.reloadTriggered)
        self.setCentralWidget(self.__configuration_panel)
        self.setWindowTitle("Configuration")
        self.activateWindow()


        # wizard_panel = WizardView()
        # wizard_panel.addGroup("Parameters")
        # wizard_panel.addItemToGroup("Parameters", "Add Summary key")
        # wizard_panel.addItemToGroup("Parameters", "Add Field parameter")
        # wizard_panel.addItemToGroup("Parameters", "Add Data Keyword")
        # wizard_panel.addGroup("Eclipse")
        # wizard_panel.addItemToGroup("Eclipse", "Setup Eclipse parameters")
        # wizard_panel.addGroup("Observations")
        # wizard_panel.addItemToGroup("Observations", "Add Observations")
        # wizard_panel.expandAll()
        # self.addDock("Wizards", wizard_panel)


    def closeEvent(self, q_close_event):
        self.__position = self.pos()
        self.__geometry = self.geometry()
        self.hide()
        q_close_event.ignore()

    def show(self):
        if self.__geometry is not None and self.__position is not None:
            self.setGeometry(self.__geometry)
            self.move(self.__position)
        QMainWindow.show(self)

    # def addDock(self, name, widget, area=Qt.RightDockWidgetArea, allowed_areas=Qt.AllDockWidgetAreas):
    #     dock_widget = QDockWidget(name)
    #     dock_widget.setObjectName("%sDock" % name)
    #     dock_widget.setWidget(widget)
    #     dock_widget.setAllowedAreas(allowed_areas)
    #
    #     self.addDockWidget(area, dock_widget)
    #
    #     self.__view_menu.addAction(dock_widget.toggleViewAction())
    #     return dock_widget
