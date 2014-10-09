from PyQt4.QtCore import QObject, pyqtSlot, QString
import math


class EnsemblePlotData(QObject):
    def __init__(self, name, case_name, parent=None):
        QObject.__init__(self, parent)

        self.__name = name
        self.__case_name = case_name

        self.__x_values = None
        self.__y_values = None
        self.__y_min_values = None
        self.__y_max_values = None

        self.__has_data = False

        self.__min_x = None
        self.__max_x = None
        self.__min_y = None
        self.__max_y = None



    def setEnsembleData(self, x_values, y_values, y_min_values, y_max_values):
        if x_values is not None and y_values is not None and y_min_values is not None and y_max_values is not None:
            self.__x_values = x_values
            self.__y_values = y_values

            self.__y_min_values = y_min_values
            self.__y_max_values = y_max_values

            self.__has_data = True


    def updateBoundaries(self, min_x, max_x, min_y, max_y):
        if min_x is not None and (self.__min_x is None or self.__min_x > min_x):
            self.__min_x = min_x

        if max_x is not None and (self.__max_x is None or self.__max_x < max_x):
            self.__max_x = max_x

        if min_y is not None and (self.__min_y is None or self.__min_y > min_y):
            self.__min_y = min_y

        if max_y is not None and (self.__max_y is None or self.__max_y < max_y):
            self.__max_y = max_y



    @pyqtSlot(result=str)
    def name(self):
        return self.__name

    @pyqtSlot(result=str)
    def caseName(self):
        return self.__case_name

    @pyqtSlot(result="QVariantList")
    def xValues(self):
        return self.__x_values

    @pyqtSlot(result="QVariantList")
    def yValues(self):
        return self.__y_values

    @pyqtSlot(result="QVariantList")
    def yMinValues(self):
        return self.__y_min_values

    @pyqtSlot(result="QVariantList")
    def yMaxValues(self):
        return self.__y_max_values

    @pyqtSlot(QString, result=int)
    def realizationCount(self):
        return len(self.__y_values)

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
        return self.hasBoundaries() and self.hasData()

    @pyqtSlot(result=bool)
    def hasBoundaries(self):
        return self.__min_x is not None and self.__max_x is not None and self.__min_y is not None and self.__max_y is not None

    @pyqtSlot(result=bool)
    def hasData(self):
        return self.__has_data



    @pyqtSlot(float, result="QVariantList")
    def xPercentile(self, percentile):
        values = []
        transposed_data = map(list, map(None, *self.__y_values))
        for row in transposed_data:
            row = sorted(row)
            values.append(self.percentile(row, percentile))

        return values


    def percentile(self, N, percent, key=lambda x:x):
        """
        Find the percentile of a list of values.

        @parameter N - is a list of values. Note N MUST BE already sorted.
        @parameter percent - a float value from 0.0 to 1.0.
        @parameter key - optional key function to compute value from each element of N.

        @return - the percentile of the values
        """
        if not N:
            return None
        k = (len(N) - 1) * percent
        f = math.floor(k)
        c = math.ceil(k)

        if f == c:
            return key(N[int(k)])

        d0 = key(N[int(f)]) * (c - k)
        d1 = key(N[int(c)]) * (k - f)

        return d0 + d1