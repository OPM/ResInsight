from math import ceil, sqrt
from PyQt4.QtCore import QObject, pyqtSlot, QString


class CaseHistogramPlotData(QObject):
    def __init__(self, case_name, report_step_time, parent=None):
        QObject.__init__(self, parent)

        self.__case_name = case_name
        self.__report_step_time = report_step_time

        self.__min = None
        self.__max = None

        self.__samples = []


    def addSample(self, sample):
        self.__samples.append(sample)

        if self.__min is None or self.__min > sample:
            self.__min = sample

        if self.__max is None or self.__max < sample:
            self.__max = sample

    def __len__(self):
        return len(self.__samples)


    @pyqtSlot(result=float)
    def min(self):
        return self.__min


    @pyqtSlot(result=float)
    def max(self):
        return self.__max


    @pyqtSlot(result="QVariantList")
    def samples(self):
        return self.__samples


class HistogramPlotData(QObject):
    def __init__(self, name, report_step_time, parent=None):
        QObject.__init__(self, parent)

        self.__name = name
        self.__report_step_time = report_step_time

        self.__min = None
        self.__max = None

        self.__case_list = []
        self.__case_histograms = {}

        self.__observation = None
        self.__observation_error = None
        self.__refcase = None


    def addCase(self, case_name):
        if not case_name in self.__case_histograms:
            self.__case_histograms[case_name] = CaseHistogramPlotData(case_name, self.__report_step_time, parent=self)
            self.__case_list.append(case_name)


    def addSample(self, case_name, sample):
        self.addCase(case_name)

        self.__case_histograms[case_name].addSample(sample)

        if self.__min is None or self.__min > sample:
            self.__min = sample

        if self.__max is None or self.__max < sample:
            self.__max = sample


    def setRefcase(self, value):
        self.__refcase = value

        if self.__min is None or self.__min > value:
            self.__min = value

        if self.__max is None or self.__max < value:
            self.__max = value

    def setObservation(self, value, error):
        self.__observation = value
        self.__observation_error = error

        if value >= 0:
            error_value = max(0, value - error)
        else:
            error_value = value - error

        if self.__min is None or self.__min > error_value:
            self.__min = error_value

        if self.__max is None or self.__max < value + error:
            self.__max = value + error


    @pyqtSlot(result=str)
    def name(self):
        return self.__name


    @pyqtSlot(result=bool)
    def hasRefcase(self):
        return self.__refcase is not None

    @pyqtSlot(result=bool)
    def hasObservation(self):
        return self.__observation is not None

    @pyqtSlot(result=float)
    def refcase(self):
        return self.__refcase

    @pyqtSlot(result=float)
    def observation(self):
        return self.__observation

    @pyqtSlot(result=float)
    def observationError(self):
        return self.__observation_error

    @pyqtSlot(QString, result=bool)
    def hasCaseHistogram(self, case_name):
        return str(case_name) in self.__case_histograms

    @pyqtSlot(QString, result=QObject)
    def caseHistogram(self, case_name):
        return self.__case_histograms[str(case_name)]

    @pyqtSlot(result=float)
    def min(self):
        return self.__min

    @pyqtSlot(result=float)
    def max(self):
        return self.__max

    @pyqtSlot(result=int)
    def reportStepTime(self):
        return self.__report_step_time

    @pyqtSlot(result=int)
    def maxCount(self):
        max_sample_count = 0
        for histogram in self.__case_histograms.values():
            max_sample_count = max(max_sample_count, len(histogram))

        return max_sample_count

    @pyqtSlot(QString, result=bool)
    def isValid(self, case_name):
        return self.hasObservation() or self.hasRefcase() or self.hasCaseHistogram(case_name)

    @pyqtSlot(result=int)
    def numberOfBins(self):
        return ceil(sqrt(self.maxCount()))

