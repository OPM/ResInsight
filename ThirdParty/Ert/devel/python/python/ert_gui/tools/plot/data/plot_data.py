from PyQt4.QtCore import QObject, pyqtSlot, QString
from ert_gui.tools.plot.data import HistogramPlotDataFactory, HistogramPlotData, ObservationPlotData, RefcasePlotData, EnsemblePlotData


class PlotData(QObject):
    def __init__(self, name, parent=None):
        QObject.__init__(self, parent)

        self.__name = name

        #: :type: ObservationPlotData
        self.__observation_data = None
        #: :type: RefcasePlotData
        self.__refcase_data = None
        #: :type: EnsemblePlotData
        self.__ensemble_data = {}

        self.__user_data = {}

        #: :type: HistogramPlotDataFactory
        self.__histogram_factory = None

        self.__unit_x = "Unknown"
        self.__unit_y = "Unknown"

        self.__min_x = None
        self.__max_x = None
        self.__min_y = None
        self.__max_y = None

        self.__use_log_scale = False

        self.__case_list = []


    def setObservationData(self, observation_data):
        if observation_data.isValid():
            observation_data.setParent(self)
            self.__observation_data = observation_data
            self.updateBoundaries(observation_data.minX(), observation_data.maxX(), observation_data.minY(), observation_data.maxY())


    def setRefcaseData(self, refcase_data):
        refcase_data.setParent(self)
        self.__refcase_data = refcase_data
        self.updateBoundaries(refcase_data.minX(), refcase_data.maxX(), refcase_data.minY(), refcase_data.maxY())

    def setHistogramFactory(self, histogram_factory):
        assert isinstance(histogram_factory, HistogramPlotDataFactory)
        self.__histogram_factory = histogram_factory

    def setUnitX(self, unit):
        self.__unit_x = unit

    def setUnitY(self, unit):
        self.__unit_y = unit

    def addEnsembleData(self, ensemble_data):
        if ensemble_data.isValid():
            ensemble_data.setParent(self)
            case_name = ensemble_data.caseName()
            self.__case_list.append(case_name)
            self.__ensemble_data[case_name] = ensemble_data
            self.updateBoundaries(ensemble_data.minX(), ensemble_data.maxX(), ensemble_data.minY(), ensemble_data.maxY())

    def setUserData(self, name, data):
        data.setParent(self)
        self.__user_data[name] = data

    def updateBoundaries(self, min_x, max_x, min_y, max_y):
        if min_x is not None and (self.__min_x is None or self.__min_x > min_x):
            self.__min_x = min_x

        if max_x is not None and (self.__max_x is None or self.__max_x < max_x):
            self.__max_x = max_x

        if min_y is not None and (self.__min_y is None or self.__min_y > min_y):
            self.__min_y = min_y

        if max_y is not None and (self.__max_y is None or self.__max_y < max_y):
            self.__max_y = max_y

    def setShouldUseLogScale(self, use_log_scale):
        self.__use_log_scale = use_log_scale

    @pyqtSlot(result=str)
    def name(self):
        """ @rtype: str """
        return self.__name

    @pyqtSlot(result=str)
    def unitX(self):
        """ @rtype: str """
        return self.__unit_x

    @pyqtSlot(result=str)
    def unitY(self):
        """ @rtype: str """
        return self.__unit_y

    @pyqtSlot(result=QObject)
    def observationData(self):
        """ @rtype: ObservationPlotData """
        return self.__observation_data

    @pyqtSlot(result=bool)
    def hasObservationData(self):
        """ @rtype: bool """
        return self.__observation_data is not None and self.__observation_data.isValid()

    @pyqtSlot(result=QObject)
    def refcaseData(self):
        """ @rtype: RefcasePlotData """
        return self.__refcase_data

    @pyqtSlot(result=bool)
    def hasRefcaseData(self):
        """ @rtype: bool """
        return self.__refcase_data is not None and self.__refcase_data.isValid()

    @pyqtSlot(QString, result=QObject)
    def ensembleData(self, case_name):
        """ @rtype: EnsemblePlotData """
        return self.__ensemble_data[str(case_name)]

    @pyqtSlot(result=bool)
    def hasEnsembleData(self):
        """ @rtype: bool """
        return len(self.__ensemble_data.keys()) > 0

    @pyqtSlot(QString, result=bool)
    def hasEnsembleDataForCase(self, case_name):
        """ @rtype: bool """
        return str(case_name) in self.__ensemble_data

    @pyqtSlot(QString, result=bool)
    def hasUserData(self, name):
        """ @rtype: bool """
        return str(name) in self.__user_data

    @pyqtSlot(QString, result=int)
    def realizationCount(self, case):
        """ @rtype: int """
        return self.__ensemble_data[str(case)].realizationCount()

    @pyqtSlot(result=bool)
    def hasHistogram(self):
        """ @rtype: bool """
        return self.__histogram_factory is not None

    @pyqtSlot(result=float)
    def minX(self):
        return self.__min_x

    @pyqtSlot(result=float)
    def maxX(self):
        return self.__max_x

    @pyqtSlot(result=float)
    def minY(self):
        return self.__min_y

    @pyqtSlot(result=float)
    def maxY(self):
        return self.__max_y

    @pyqtSlot(result=bool)
    def isValid(self):
        return self.hasBoundaries() and (self.hasObservationData() or self.hasRefcaseData() or self.hasEnsembleData() or self.hasHistogram())

    @pyqtSlot(result=bool)
    def hasBoundaries(self):
        return self.__min_x is not None and self.__max_x is not None and self.__min_y is not None and self.__max_y is not None

    @pyqtSlot(result="QStringList")
    def caseList(self):
        return self.__case_list

    @pyqtSlot(result=bool)
    def shouldUseLogScale(self):
        return self.__use_log_scale

    @pyqtSlot(int, result=QObject)
    def histogramData(self, report_step_time):
        if self.__histogram_factory is not None:
            data = self.__histogram_factory.getHistogramData(report_step_time)
            data.setParent(self)

            return data
        return None

    @pyqtSlot("QString", result=QObject)
    def getUserData(self, name):
        return self.__user_data[str(name)]

