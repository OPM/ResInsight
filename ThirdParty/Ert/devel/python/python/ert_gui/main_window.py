from PyQt4.QtCore import QSettings, Qt
from PyQt4.QtGui import QMainWindow, QTabWidget, qApp
from ert_gui.widgets.help_dock import HelpDock


class GertMainWindow(QMainWindow):
    """An application (window widget) with a list of "tasks" on the left side and a panel on the right side"""

    def __init__(self):
        """Constructor"""
        QMainWindow.__init__(self)

        self.resize(900, 700)
        self.setWindowTitle('gERT')

        self.tabs = QTabWidget()
        self.setCentralWidget(self.tabs)

        self.help_dock = HelpDock.getInstance()
        self.addDockWidget(Qt.RightDockWidgetArea, self.help_dock)


        self.__createMenu()
        self.save_function = None
        self.__fetchSettings()


    def setSaveFunction(self, save_function):
        """Set the function to be called when the save menu choice is selected."""
        self.save_function = save_function

    def addTab(self, name, tab_widget):
        self.tabs.addTab(tab_widget, name)

    def __save(self):
        if not self.save_function is None:
            self.save_function()

    def __createMenu(self):
        file_menu = self.menuBar().addMenu("&File")
        file_menu.addAction("Save Config File", self.__save)
        file_menu.addAction("Close", self.__quit)

        view_menu = self.menuBar().addMenu("&View")
        view_menu.addAction(self.help_dock.toggleViewAction())

    def __quit(self):
        self.__saveSettings()
        qApp.quit()

    def __saveSettings(self):
        settings = QSettings("Statoil", "ErtGui")
        settings.setValue("geometry", self.saveGeometry())
        settings.setValue("windowState", self.saveState())

    def closeEvent(self, event):
        #Use QT settings saving mechanism
        #settings stored in ~/.config/Statoil/ErtGui.conf
        self.__saveSettings()
        QMainWindow.closeEvent(self, event)

    def __fetchSettings(self):
        settings = QSettings("Statoil", "ErtGui")
        self.restoreGeometry(settings.value("geometry").toByteArray())
        self.restoreState(settings.value("windowState").toByteArray())


