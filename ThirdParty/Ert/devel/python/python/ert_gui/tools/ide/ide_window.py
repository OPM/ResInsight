from PyQt4.QtCore import Qt, pyqtSignal
from PyQt4.QtGui import QMainWindow, QDockWidget
from ert_gui.ide.wizards import WizardView
from ert_gui.pages.configuration_panel import ConfigurationPanel
from ert_gui.widgets.help_dock import HelpDock


class IdeWindow(QMainWindow):
    reloadTriggered = pyqtSignal()

    def __init__(self, path, parent):
        QMainWindow.__init__(self, parent)

        self.resize(900, 900)

        self.__geometry = None

        self.__configuration_panel = ConfigurationPanel(path)
        self.__configuration_panel.reloadApplication.connect(self.reloadTriggered)
        self.setCentralWidget(self.__configuration_panel)
        self.setWindowTitle("Configuration")
        self.activateWindow()

        self.__view_menu = self.menuBar().addMenu("&View")

        wizard_panel = WizardView()
        wizard_panel.addGroup("Parameters")
        wizard_panel.addItemToGroup("Parameters", "Add Summary key")
        wizard_panel.addItemToGroup("Parameters", "Add Field parameter")
        wizard_panel.addItemToGroup("Parameters", "Add Data Keyword")
        wizard_panel.addGroup("Eclipse")
        wizard_panel.addItemToGroup("Eclipse", "Setup Eclipse parameters")
        wizard_panel.addGroup("Observations")
        wizard_panel.addItemToGroup("Observations", "Add Observations")
        wizard_panel.expandAll()
        self.addDock("Wizards", wizard_panel)

        self.__help_dock = HelpDock.getInstance() # todo Turn HelpDock into a panel
        help_dock = self.addDockWidget(Qt.RightDockWidgetArea, self.__help_dock)


    def closeEvent(self, q_close_event):
        self.__geometry = self.geometry()
        self.hide()
        q_close_event.ignore()

    def show(self):
        if not self.__geometry is None:
            self.setGeometry(self.__geometry)
        QMainWindow.show(self)

    def addDock(self, name, widget, area=Qt.RightDockWidgetArea, allowed_areas=Qt.AllDockWidgetAreas):
        dock_widget = QDockWidget(name)
        dock_widget.setObjectName("%sDock" % name)
        dock_widget.setWidget(widget)
        dock_widget.setAllowedAreas(allowed_areas)

        self.addDockWidget(area, dock_widget)
        self.__view_menu.addAction(dock_widget.toggleViewAction())
        return dock_widget
