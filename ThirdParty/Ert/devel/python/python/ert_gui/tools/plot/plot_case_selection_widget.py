from PyQt4.QtCore import pyqtSignal, QSignalMapper
from PyQt4.QtGui import QWidget, QVBoxLayout, QHBoxLayout, QToolButton, QComboBox, QPushButton
from ert_gui.tools.plot import PlotCaseModel
from ert_gui.widgets import util


class CaseSelectionWidget(QWidget):

    caseSelectionChanged = pyqtSignal()

    def __init__(self, current_case):
        QWidget.__init__(self)

        self.__model = PlotCaseModel()

        self.__signal_mapper = QSignalMapper(self)
        self.__case_selectors = {}
        self.__case_selectors_order = []

        layout = QVBoxLayout()

        add_button_layout = QHBoxLayout()
        button = QPushButton(util.resourceIcon("ide/small/add"), "Add case to plot")
        button.clicked.connect(self.addCaseSelector)

        add_button_layout.addStretch()
        add_button_layout.addWidget(button)
        add_button_layout.addStretch()

        layout.addLayout(add_button_layout)

        self.__case_layout = QVBoxLayout()
        self.__case_layout.setMargin(0)
        layout.addLayout(self.__case_layout)

        self.addCaseSelector(disabled=True, current_case=current_case)
        layout.addStretch()

        self.setLayout(layout)

        self.__signal_mapper.mapped[QWidget].connect(self.removeWidget)


    def getPlotCaseNames(self):
        return [str(self.__case_selectors[widget].currentText()) for widget in self.__case_selectors_order]


    def addCaseSelector(self, disabled=False, current_case=None):
        if len(self.__case_selectors_order) == 5:
            return

        widget = QWidget()

        layout = QHBoxLayout()
        layout.setMargin(0)
        widget.setLayout(layout)

        combo = QComboBox()
        combo.setSizeAdjustPolicy(QComboBox.AdjustToMinimumContentsLengthWithIcon)
        combo.setMinimumContentsLength(20)
        combo.setModel(self.__model)

        if current_case is not None:
            index = 0
            for item in self.__model:
                if item == current_case:
                    combo.setCurrentIndex(index)
                    break
                index += 1

        combo.currentIndexChanged.connect(self.caseSelectionChanged.emit)



        layout.addWidget(combo, 1)

        button = QToolButton()
        button.setAutoRaise(True)
        button.setDisabled(disabled)
        button.setIcon(util.resourceIcon("ide/small/delete"))
        button.clicked.connect(self.__signal_mapper.map)

        layout.addWidget(button)

        self.__case_selectors[widget] = combo
        self.__case_selectors_order.append(widget)
        self.__signal_mapper.setMapping(button, widget)

        self.__case_layout.addWidget(widget)

        self.caseSelectionChanged.emit()



    def removeWidget(self, widget):
        self.__case_layout.removeWidget(widget)
        del self.__case_selectors[widget]
        self.__case_selectors_order.remove(widget)
        widget.setParent(None)
        self.caseSelectionChanged.emit()

