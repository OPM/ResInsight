import json
import os
from PyQt4.QtCore import QObject, pyqtSlot, pyqtSignal, QUrl, QVariant
from PyQt4.QtWebKit import QWebPage
from ert.util import CTime
from ert_gui.tools.plot.data import PlotData

class PlotWebPage(QWebPage):
    def __init__(self, name):
        QWebPage.__init__(self)
        self.name = name

    def javaScriptConsoleMessage(self, message, line_number, source_id):
        print("[%s] Source: %s at line: %d -> %s" % (self.name, source_id, line_number, message))


class PlotBridge(QObject):
    plotReady = pyqtSignal()
    renderingFinished = pyqtSignal()

    def __init__(self, web_page, plot_url):
        QObject.__init__(self)
        assert isinstance(web_page, QWebPage)

        self.__web_page = web_page
        self.__ready = False
        self.__html_ready = False
        self.__data = PlotData("invalid", parent=self)
        self.__size = None
        self.__temporary_data_object = None

        self.applyContextObject()

        root_path = os.getenv("ERT_SHARE_PATH")
        path = os.path.join(root_path, plot_url)
        self.__web_page.mainFrame().load(QUrl("file://%s" % path))
        self.__web_page.loadFinished.connect(self.loadFinished)


    def applyContextObject(self):
        self.__web_page.mainFrame().addToJavaScriptWindowObject("plot_data_source", self)

    def updatePlotSize(self, size):
        self.__size = size
        if self.isReady():
            self.__web_page.mainFrame().evaluateJavaScript("setSize(%d,%d);" % (size.width(), size.height()))
            self.renderNow()

    def supportsPlotProperties(self, time=False, value=False, depth=False, index=False, histogram=False, pca=False):
        time = str(time).lower()
        value = str(value).lower()
        depth = str(depth).lower()
        index = str(index).lower()
        histogram = str(histogram).lower()
        pca = str(pca).lower()
        return self.__web_page.mainFrame().evaluateJavaScript("supportsPlotProperties(%s,%s,%s,%s,%s,%s);" % (time, value, depth, index, histogram, pca)).toBool()

    def setPlotData(self, data):
        self.__data = data
        self.__web_page.mainFrame().evaluateJavaScript("updatePlot();")

    def setReportStepTime(self, report_step_time):
        if report_step_time is None:
            report_step_time = "null"

        if isinstance(report_step_time, CTime):
            report_step_time = report_step_time.ctime()

        self.__web_page.mainFrame().evaluateJavaScript("setReportStepTime(%s);" % report_step_time)


    def setScales(self, x_min, x_max, y_min, y_max):
        if x_min is None:
            x_min = "null"

        if x_max is None:
            x_max = "null"

        if y_min is None:
            y_min = "null"

        if y_max is None:
            y_max = "null"

        if isinstance(x_min, CTime):
            x_min = x_min.ctime()

        if isinstance(x_max, CTime):
            x_max = x_max.ctime()

        if isinstance(y_min, CTime):
            y_min = y_min.ctime()

        if isinstance(y_max, CTime):
            y_max = y_max.ctime()

        scales = (x_min, x_max, y_min, y_max)
        self.__web_page.mainFrame().evaluateJavaScript("setScales(%s,%s,%s,%s);" % scales)

    @pyqtSlot(result=QObject)
    def getPlotData(self):
        return self.__data


    @pyqtSlot()
    def htmlInitialized(self):
        # print("[%s] Initialized!" % self.__name)
        self.__html_ready = True
        self.checkStatus()

    def loadFinished(self, ok):
        self.__ready = True
        self.checkStatus()

    def checkStatus(self):
        if self.__ready and self.__html_ready:
            # print("[%s] Ready!" % self.__name)
            self.plotReady.emit()
            if self.__size is not None:
                self.updatePlotSize(self.__size)

    def isReady(self):
        return self.__ready and self.__html_ready

    def getPrintWidth(self):
        return self.__web_page.mainFrame().evaluateJavaScript("getPrintWidth();").toInt()[0]

    def getPrintHeight(self):
        return self.__web_page.mainFrame().evaluateJavaScript("getPrintHeight();").toInt()[0]

    def getPage(self):
        return self.__web_page

    def setCustomSettings(self, settings):
        json_settings = json.dumps(settings)
        self.__web_page.mainFrame().evaluateJavaScript("setCustomSettings(%s);" % json_settings)

    def renderNow(self):
        self.__web_page.mainFrame().evaluateJavaScript("renderNow()")


    def getPlotTitle(self):
        return self.__web_page.mainFrame().evaluateJavaScript("getPlotTitle();" ).toString()


    def xAxisType(self):
        """ @rtype: str """
        axis_type = self.__web_page.mainFrame().evaluateJavaScript("xAxisType();")

        if axis_type.isNull():
            return None

        return str(axis_type.toString())


    def yAxisType(self):
        """ @rtype: str """
        axis_type = self.__web_page.mainFrame().evaluateJavaScript("yAxisType();")

        if axis_type.isNull():
            return None

        return str(axis_type.toString())


    def isReportStepCapable(self):
        """ @rtype: bool """
        return self.__web_page.mainFrame().evaluateJavaScript("isReportStepCapable();" ).toBool()


    @pyqtSlot(result=QObject)
    def getTemporaryData(self):
        return self.__temporary_data_object


    def getXScales(self, data):
        """ @rtype: (float, float) """
        self.__temporary_data_object = data
        x_min = self.__web_page.mainFrame().evaluateJavaScript("getXMin();")
        x_max = self.__web_page.mainFrame().evaluateJavaScript("getXMax();")

        if x_min.isNull():
            x_min = None
        elif x_min.typeName() == "double":
            x_min = x_min.toDouble()[0]
        else:
            raise TypeError("Unknown type %s for x_min" % x_min.typeName())

        if x_max.isNull():
            x_max = None
        elif x_max.typeName() == "double":
            x_max = x_max.toDouble()[0]
        else:
            raise TypeError("Unknown type %s for x_max" % x_max.typeName())

        return x_min, x_max


    def getYScales(self, data):
        """ @rtype: (float, float) """
        self.__temporary_data_object = data
        y_min = self.__web_page.mainFrame().evaluateJavaScript("getYMin();")
        y_max = self.__web_page.mainFrame().evaluateJavaScript("getYMax();")

        if y_min.isNull():
            y_min = None
        elif y_min.typeName() == "double":
            y_min = y_min.toDouble()[0]
        else:
            raise TypeError("Unknown type %s for y_min" % y_min.typeName())

        if y_max.isNull():
            y_max = None
        elif y_max.typeName() == "double":
            y_max = y_max.toDouble()[0]
        else:
            raise TypeError("Unknown type %s for y_max" % y_max.typeName())

        return y_min, y_max





