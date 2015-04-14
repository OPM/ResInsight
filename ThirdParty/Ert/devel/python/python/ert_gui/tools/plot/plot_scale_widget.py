from PyQt4.QtCore import pyqtSignal, Qt
from PyQt4.QtGui import QWidget, QVBoxLayout, QCheckBox, QDoubleSpinBox, QSpinBox, QStackedWidget, QSizePolicy, QLabel

from ert.util import CTime
from ert_gui.models.connectors.plot import ReportStepsModel
from ert_gui.widgets.list_spin_box import ListSpinBox


class PlotScalesWidget(QWidget):
    plotScaleChanged = pyqtSignal()

    def __init__(self, type_key, title, select_min_time_value=False):
        QWidget.__init__(self)

        self.__type_key = type_key
        self.__type = None

        self.__double_spinner = self.createDoubleSpinner(minimum=-1e15, maximum=1e15)
        self.__integer_spinner = self.createIntegerSpinner(minimum=0, maximum=1e10)

        self.__time_map = ReportStepsModel().getList()
        self.__time_index_map = {}
        for index in range(len(self.__time_map)):
            time = self.__time_map[index]
            self.__time_index_map[time] = index

        self.__time_spinner = self.createTimeSpinner(select_minimum_value=select_min_time_value)

        layout = QVBoxLayout()
        self.setLayout(layout)

        self.__label = QLabel(title)
        self.__label.setAlignment(Qt.AlignHCenter)

        self.__stack = QStackedWidget()
        self.__stack.setSizePolicy(QSizePolicy(QSizePolicy.Preferred))
        self.__stack.addWidget(self.__integer_spinner)
        self.__stack.addWidget(self.__double_spinner)
        self.__stack.addWidget(self.__time_spinner)

        layout.addWidget(self.__stack)
        layout.addWidget(self.__label)

        self.setLayout(layout)

    def createDoubleSpinner(self, minimum, maximum):
        spinner = QDoubleSpinBox()
        spinner.setSizePolicy(QSizePolicy.Ignored, QSizePolicy.Ignored)
        spinner.setMinimumWidth(105)
        spinner.setRange(minimum, maximum)
        spinner.setKeyboardTracking(False)
        spinner.setDecimals(8)

        spinner.editingFinished.connect(self.plotScaleChanged)
        spinner.valueChanged.connect(self.plotScaleChanged)

        return spinner

    def createIntegerSpinner(self, minimum, maximum):
        spinner = QSpinBox()
        spinner.setMinimumWidth(75)
        spinner.setRange(minimum, maximum)
        spinner.setKeyboardTracking(False)

        spinner.editingFinished.connect(self.plotScaleChanged)
        spinner.valueChanged.connect(self.plotScaleChanged)

        return spinner

    def createTimeSpinner(self, select_minimum_value):
        def converter(item):
            return "%s" % (str(item.date()))

        spinner = ListSpinBox(self.__time_map)
        spinner.setMinimumWidth(75)

        if select_minimum_value:
            spinner.setValue(0)

        spinner.valueChanged[int].connect(self.plotScaleChanged)
        spinner.editingFinished.connect(self.plotScaleChanged)
        spinner.setStringConverter(converter)

        return spinner


    def getValue(self):
        if self.__type is int:
            return self.__integer_spinner.value()
        elif self.__type is float:
            return self.__double_spinner.value()
        elif self.__type is CTime:
            index = self.__time_spinner.value()
            return self.__time_map[index]
        else:
            raise TypeError("Unsupported spinner type: %s" % self.__type)


    def setValue(self, value):
        if value is not None:
            if self.__type is int:
                self.__integer_spinner.setValue(int(value))
            elif self.__type is float:
                self.__double_spinner.setValue(value)
            elif self.__type is CTime:
                index = self.__time_index_map[value]
                self.__time_spinner.setValue(index)
            else:
                raise TypeError("Unsupported spinner type: %s" % self.__type)


    def setFontSize(self, size):
        font = self.__double_spinner.font()
        font.setPointSize(size)
        self.__double_spinner.setFont(font)

        font = self.__integer_spinner.font()
        font.setPointSize(size)
        self.__integer_spinner.setFont(font)

        font = self.__time_spinner.font()
        font.setPointSize(size)
        self.__time_spinner.setFont(font)

        font = self.__label.font()
        font.setPointSize(size)
        self.__label.setFont(font)


    def setType(self, spinner_type):
        self.__type = spinner_type
        if spinner_type is int:
            self.__stack.setCurrentWidget(self.__integer_spinner)
        elif spinner_type is float:
            self.__stack.setCurrentWidget(self.__double_spinner)
        elif spinner_type is CTime:
            self.__stack.setCurrentWidget(self.__time_spinner)
        else:
            raise TypeError("Unsupported spinner type: %s" % spinner_type)


    def getType(self):
        return self.__type
