from PyQt4.QtCore import QObject, pyqtSlot


class ObservationPlotData(QObject):
    def __init__(self, name, parent=None):
        QObject.__init__(self, parent)

        self.__name = name
        self.__x_values = []
        self.__y_values = []
        self.__std_values = []
        self.__is_continuous = True
        self.__has_data = False

        self.__min_x = None
        self.__max_x = None
        self.__min_y = None
        self.__max_y = None


    def setObservationData(self, x_values, y_values, std_values, continuous):
        if x_values is not None and y_values is not None and std_values is not None:
            self.__x_values = x_values
            self.__y_values = y_values
            self.__std_values = std_values
            self.__is_continuous = continuous
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

    @pyqtSlot(result="QVariantList")
    def xValues(self):
        return self.__x_values

    @pyqtSlot(result="QVariantList")
    def yValues(self):
        return self.__y_values

    @pyqtSlot(result="QVariantList")
    def stdValues(self):
        return self.__std_values

    @pyqtSlot(result=bool)
    def isContinuous(self):
        return self.__is_continuous

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
