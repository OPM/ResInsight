from PyQt4.QtCore import Qt, pyqtSignal
from PyQt4.QtGui import QToolBar
from ert.util import CTime
from ert_gui.tools.plot import ReportStepWidget, PlotScalesWidget
from ert_gui.widgets import util


class PlotToolBar(QToolBar):
    FONT_SIZE = 11

    exportClicked = pyqtSignal()
    resetScalesClicked = pyqtSignal()
    reportStepChanged = pyqtSignal()
    plotScalesChanged = pyqtSignal()

    def __init__(self):
        QToolBar.__init__(self, "PlotTools")

        self.setObjectName("PlotToolBar")
        self.setToolButtonStyle(Qt.ToolButtonTextUnderIcon)

        self.__reset_scales = self.createAction("Reset Scales", util.resourceIcon("ide/transform_scale"))
        self.__reset_scales.triggered.connect(self.resetScalesClicked)

        self.__x_min, self.__x_min_action = self.addScaler("x_min", "X Minimum", spinner_type=CTime, select_min_time_value=True)
        self.__x_max, self.__x_max_action = self.addScaler("x_max", "X Maximum", spinner_type=CTime)
        self.__y_min, self.__y_min_action = self.addScaler("y_min", "Y Minimum", spinner_type=float, select_min_time_value=True)
        self.__y_max, self.__y_max_action = self.addScaler("y_max", "Y Maximum", spinner_type=float)

        self.__report_step_widget = ReportStepWidget()
        self.__report_step_widget.reportStepTimeSelected.connect(self.reportStepChanged)
        self.__report_step_widget.setFontSize(PlotToolBar.FONT_SIZE)

        self.__report_step_widget_action = self.addWidget(self.__report_step_widget)

        self.addSeparator()

        export_action = self.createAction("Export Plot", util.resourceIcon("ide/table_export"))
        export_action.triggered.connect(self.exportClicked)


    def createAction(self, title, icon):
        action = self.addAction(title)
        action.setIcon(icon)

        w = self.widgetForAction(action)
        font = w.font()
        font.setPointSize(PlotToolBar.FONT_SIZE)
        w.setFont(font)

        return action

    def addScaler(self, type_key, title, spinner_type, select_min_time_value=False):
        scaler = PlotScalesWidget(type_key, title, select_min_time_value=select_min_time_value)
        scaler.setFontSize(PlotToolBar.FONT_SIZE)
        scaler.plotScaleChanged.connect(self.plotScalesChanged)
        scaler.setType(spinner_type)

        action = self.addWidget(scaler)

        return scaler, action


    def setToolBarOptions(self, x_type, y_type, report_step_capable):
        self.blockSignals(True)
        self.__x_min_action.setVisible(x_type is not None)
        self.__x_max_action.setVisible(x_type is not None)

        self.__y_min_action.setVisible(y_type is not None)
        self.__y_max_action.setVisible(y_type is not None)

        if x_type is not None:
            self.__x_min.setType(x_type)
            self.__x_max.setType(x_type)

        if y_type is not None:
            self.__y_min.setType(y_type)
            self.__y_max.setType(y_type)

        self.__report_step_widget_action.setVisible(report_step_capable)
        self.blockSignals(False)


    def getXScales(self):
        return (self.__x_min.getValue()), (self.__x_max.getValue())


    def getYScales(self):
        return (self.__y_min.getValue()), (self.__y_max.getValue())


    def getReportStep(self):
        """ @rtype: CTime """
        return self.__report_step_widget.getSelectedValue()


    def setScales(self, x_min, x_max, y_min, y_max):
        self.blockSignals(True)

        self.__x_min.setValue(x_min)
        self.__x_max.setValue(x_max)
        self.__y_min.setValue(y_min)
        self.__y_max.setValue(y_max)

        self.blockSignals(False)
