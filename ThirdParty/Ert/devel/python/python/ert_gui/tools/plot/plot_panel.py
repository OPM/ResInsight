import json
import os
from PyQt4.QtCore import QUrl, Qt, pyqtSlot, pyqtSignal
from PyQt4.QtGui import QWidget, QGridLayout, QPainter
from PyQt4.QtWebKit import QWebView, QWebPage, QWebSettings


class PlotWebPage(QWebPage):
    def __init__(self, name):
        QWebPage.__init__(self)
        self.name = name

    def javaScriptConsoleMessage(self, message, line_number, source_id):
        print("[%s] Source: %s at line: %d -> %s" % (self.name, source_id, line_number, message))


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



class PlotPanel(QWidget):
    plotReady = pyqtSignal()

    def __init__(self, name, plot_url):
        QWidget.__init__(self)

        self.__name = name
        self.__ready = False
        self.__html_ready = False
        self.__data = []
        root_path = os.getenv("ERT_SHARE_PATH")
        path = os.path.join(root_path, plot_url)

        layout = QGridLayout()

        self.web_view = PlotWebView(name)
        self.applyContextObject()
        # self.web_view.page().mainFrame().javaScriptWindowObjectCleared.connect(self.applyContextObject)
        self.web_view.loadFinished.connect(self.loadFinished)

        layout.addWidget(self.web_view)

        self.setLayout(layout)

        self.web_view.setUrl(QUrl("file://%s" % path))


    @pyqtSlot(result=str)
    def getPlotData(self):
        return json.dumps(self.__data)


    @pyqtSlot()
    def htmlInitialized(self):
        # print("[%s] Initialized!" % self.__name)
        self.__html_ready = True
        self.checkStatus()


    def setPlotData(self, data):
        self.__data = data
        self.web_view.page().mainFrame().evaluateJavaScript("updatePlot();")


    def applyContextObject(self):
        self.web_view.page().mainFrame().addToJavaScriptWindowObject("plot_data_source", self)


    def loadFinished(self, ok):
        self.__ready = True
        self.checkStatus()

    def checkStatus(self):
        if self.__ready and self.__html_ready:
            # print("[%s] Ready!" % self.__name)
            self.plotReady.emit()
            self.updatePlotSize()


    def isReady(self):
        return self.__ready and self.__html_ready


    def resizeEvent(self, event):
        QWidget.resizeEvent(self, event)

        if self.isReady():
            self.updatePlotSize()


    def updatePlotSize(self):
        size = self.size()
        self.web_view.page().mainFrame().evaluateJavaScript("setSize(%d,%d);" % (size.width(), size.height()))






