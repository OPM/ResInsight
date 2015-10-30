from PyQt4.QtCore import pyqtSignal
from PyQt4.QtGui import QWidget, QVBoxLayout, QCheckBox, QColor

from ert_gui.tools.plot import ColorChooser

class CustomizePlotWidget(QWidget):

    customPlotSettingsChanged = pyqtSignal()

    def __init__(self):
        QWidget.__init__(self)
        self.__custom = { }

        self.__layout = QVBoxLayout()

        self.addCheckBox("show_observations", "Show observations", True)
        self.addCheckBox("show_refcase", "Show refcase", True)
        self.addCheckBox("show_legend", "Show legend", True)
        self.addCheckBox("show_grid", "Show grid", True)
        self.__layout.addSpacing(20)

        # self.addColorChooser("observation", "Observation", QColor(0, 0, 0, 255))
        # self.addColorChooser("observation_area", "Observation Error", QColor(0, 0, 0, 38))
        # self.addColorChooser("observation_error_bar", "Observation Error Bar", QColor(0, 0, 0, 255))
        # self.addColorChooser("refcase", "Refcase", QColor(0, 0, 0, 178))
        # self.addColorChooser("ensemble_1", "Case #1", QColor(56, 108, 176, 204))
        # self.addColorChooser("ensemble_2", "Case #2", QColor(127, 201, 127, 204))
        # self.addColorChooser("ensemble_3", "Case #3", QColor(253, 192, 134, 204))
        # self.addColorChooser("ensemble_4", "Case #4", QColor(240, 2, 127, 204))
        # self.addColorChooser("ensemble_5", "Case #5", QColor(191, 91, 23, 204))

        self.__layout.addStretch()

        self.setLayout(self.__layout)

    def emitChange(self):
        self.customPlotSettingsChanged.emit()

    def addCheckBox(self, name, description, default_value):
        checkbox = QCheckBox(description)
        checkbox.setChecked(default_value)
        self.__custom[name] = default_value

        def toggle(checked):
            self.__custom[name] = checked
            self.emitChange()

        checkbox.toggled.connect(toggle)

        self.__layout.addWidget(checkbox)

    def createJSColor(self, color):
        return "rgba(%d, %d, %d, %f)" % (color.red(), color.green(), color.blue(), color.alphaF())

    def getCustomSettings(self):
        return self.__custom
    
    def addColorChooser(self, name, label, default_color):
        color_chooser = ColorChooser(label, default_color)
        self.__custom[name] = self.createJSColor(default_color)

        def colorChanged(color):
            self.__custom[name] = self.createJSColor(color)
            self.emitChange()

        color_chooser.colorChanged.connect(colorChanged)

        self.__layout.addWidget(color_chooser)


