from PyQt4.QtCore import Qt, pyqtSignal
from PyQt4.QtGui import QWidget, QGridLayout, QPainter, QShortcut, QMainWindow
from PyQt4.QtWebKit import QWebView, QWebSettings, QWebInspector

from ert_gui.tools.plot import PlotBridge
from ert_gui.tools.plot.plot_bridge import PlotWebPage


class PlotWebView(QWebView):
    def __init__(self, name):
        QWebView.__init__(self)
        self.setPage(PlotWebPage(name))
        self.setRenderHint(QPainter.Antialiasing, True)
        self.setContextMenuPolicy(Qt.NoContextMenu)
        self.settings().setAttribute(QWebSettings.JavascriptEnabled, True)
        self.settings().setAttribute(QWebSettings.LocalContentCanAccessFileUrls, True)
        self.settings().setAttribute(QWebSettings.LocalContentCanAccessRemoteUrls, True)
        self.settings().clearMemoryCaches()

        self.__inspector_window = None

        shortcut = QShortcut(self)
        shortcut.setKey(Qt.Key_F12)
        shortcut.activated.connect(self.toggleInspector)


    def toggleInspector(self):
        if self.__inspector_window is None:
            self.settings().setAttribute(QWebSettings.DeveloperExtrasEnabled, True)
            web_inspector = QWebInspector()
            web_inspector.setPage(self.page())

            self.__inspector_window = QMainWindow(self)
            self.__inspector_window.setCentralWidget(web_inspector)
            self.__inspector_window.resize(900, 600)
            self.__inspector_window.setVisible(False)

        self.__inspector_window.setVisible(not self.__inspector_window.isVisible())


class PlotPanel(QWidget):
    plotReady = pyqtSignal()

    def __init__(self, name, debug_name, plot_url):
        QWidget.__init__(self)

        self.__name = name
        self.__debug_name = debug_name
        self.__plot_url = plot_url

        layout = QGridLayout()

        self.web_view = PlotWebView(debug_name)

        layout.addWidget(self.web_view)
        self.setLayout(layout)

        self.__plot_is_visible = True
        self.__plot_bridge = PlotBridge(self.getWebView().page(), plot_url)
        self.__plot_bridge.plotReady.connect(self.plotReady)


    def getName(self):
        return self.__name

    def getUrl(self):
        return self.__plot_url

    def getWebView(self):
        return self.web_view

    def setSettings(self, settings):
        if self.isPlotVisible():
            self.__plot_bridge.setPlotSettings(settings)

    def isReady(self):
        return self.__plot_bridge.isReady()


    def resizeEvent(self, event):
        QWidget.resizeEvent(self, event)
        if self.isPlotVisible():
            self.__plot_bridge.updatePlotSize(size = self.size())


    def supportsPlotProperties(self, time=False, value=False, depth=False, index=False, histogram=False, pca=False):
        return self.__plot_bridge.supportsPlotProperties(time, value, depth, index, histogram, pca)

    def isPlotVisible(self):
        return self.__plot_is_visible

    def setPlotIsVisible(self, visible):
        self.__plot_is_visible = visible

    def getPlotBridge(self):
        """ @rtype: PlotBridge """
        return self.__plot_bridge

    def renderNow(self):
        if self.isPlotVisible():
            self.__plot_bridge.renderNow()


    def xAxisType(self):
        """ @rtype: str """
        return self.__plot_bridge.xAxisType()


    def yAxisType(self):
        """ @rtype: str """
        return self.__plot_bridge.yAxisType()

    def getXScales(self, data):
        """ @rtype: (float, float) """
        return self.__plot_bridge.getXScales(data)

    def getYScales(self, data):
        """ @rtype: (float, float) """
        return self.__plot_bridge.getYScales(data)


    def isReportStepCapable(self):
        """ @rtype: bool """
        return self.__plot_bridge.isReportStepCapable()