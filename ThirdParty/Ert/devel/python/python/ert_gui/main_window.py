from PyQt4.QtCore import QSettings, Qt
from PyQt4.QtGui import QMainWindow, qApp, QWidget, QVBoxLayout


class GertMainWindow(QMainWindow):
    def __init__(self):
        QMainWindow.__init__(self)

        self.tools = {}

        self.resize(300, 700)
        self.setWindowTitle('gERT')

        self.central_widget = QWidget()
        self.central_layout = QVBoxLayout()
        self.central_widget.setLayout(self.central_layout)

        self.setCentralWidget(self.central_widget)

        self.toolbar = self.addToolBar("Tools")
        self.toolbar.setObjectName("Toolbar")
        self.toolbar.setToolButtonStyle(Qt.ToolButtonTextUnderIcon)

        # configure_action = toolbar.addAction(util.resourceIcon("ide/cog_edit"), "Configure")
        #
        # plot_action = toolbar.addAction(util.resourceIcon("ide/chart_curve_add"), "Plot")
        # save_action.triggered.connect(self.save)

        # reload_action.triggered.connect(self.reload)

        # toolbar.addSeparator()
        #
        # stretchy_separator = QWidget()
        # stretchy_separator.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        # toolbar.addWidget(stretchy_separator)

        #


        self.__createMenu()
        self.__fetchSettings()


    def addTool(self, tool):
        tool.setParent(self)
        self.tools[tool.getName()] = tool
        action = self.toolbar.addAction(tool.getIcon(), tool.getName())
        action.setIconText(tool.getName())
        action.setEnabled(tool.isEnabled())
        action.triggered.connect(tool.trigger)


    def __createMenu(self):
        file_menu = self.menuBar().addMenu("&File")
        file_menu.addAction("Close", self.__quit)


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


    def setWidget(self, widget):
        self.central_layout.addWidget(widget)



