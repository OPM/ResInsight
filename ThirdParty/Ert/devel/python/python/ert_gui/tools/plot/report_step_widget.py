from PyQt4.QtCore import pyqtSignal, Qt
from PyQt4.QtGui import QWidget, QVBoxLayout, QLabel
from ert_gui.models.connectors.plot.report_steps import ReportStepsModel
from ert_gui.widgets.list_spin_box import ListSpinBox


class ReportStepWidget(QWidget):
    reportStepTimeSelected = pyqtSignal(int)
    def __init__(self):
        QWidget.__init__(self)

        layout = QVBoxLayout()
        self.setLayout(layout)

        def converter(item):
            return "%s" % (str(item.date()))

        self.__items = ReportStepsModel().getList()
        self.__time_spinner = ListSpinBox(self.__items)
        self.__time_spinner.valueChanged[int].connect(self.valueSelected)
        self.__time_spinner.setStringConverter(converter)
        layout.addWidget(self.__time_spinner)

        self.__label = QLabel("Report Step")

        layout.addWidget(self.__label, 0, Qt.AlignHCenter)
        layout.addStretch()


    def valueSelected(self, index):
        self.reportStepTimeSelected.emit(self.__items[index])

    def getSelectedValue(self):
        """ @rtype: CTime """
        index = self.__time_spinner.value()
        return self.__items[index]

    def setFontSize(self, size):
        font = self.__time_spinner.font()
        font.setPointSize(size)
        self.__time_spinner.setFont(font)

        font = self.__label.font()
        font.setPointSize(size)
        self.__label.setFont(font)
