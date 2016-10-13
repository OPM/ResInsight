import sys
import traceback

from PyQt4.QtCore import Qt, pyqtSignal
from PyQt4.QtGui import QWidget, QVBoxLayout, QAction

from matplotlib.figure import Figure
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas, NavigationToolbar2QT

from ert_gui.ertwidgets import resourceIcon


class CustomNavigationToolbar(NavigationToolbar2QT):
    customizationTriggered = pyqtSignal()

    def __init__(self, canvas, parent, coordinates=True):
        super(CustomNavigationToolbar, self).__init__(canvas, parent, coordinates)

        gear = resourceIcon("ide/cog_edit.png")
        customize_action = QAction(gear, "Customize", self)
        customize_action.setToolTip("Customize plot settings")
        customize_action.triggered.connect(self.customizationTriggered)

        for action in self.actions():
            if str(action.text()).lower() == "subplots":
                self.removeAction(action)

            if str(action.text()).lower() == "customize":
                self.insertAction(action, customize_action)
                self.removeAction(action)
                break



class PlotWidget(QWidget):
    customizationTriggered = pyqtSignal()

    def __init__(self, name, plotFunction, plot_condition_function_list, plotContextFunction, parent=None):
        QWidget.__init__(self, parent)

        self._name = name
        self._plotFunction = plotFunction
        self._plotContextFunction = plotContextFunction
        self._plot_conditions = plot_condition_function_list
        """:type: list of functions """

        self._figure = Figure()
        self._figure.set_tight_layout(True)
        self._canvas = FigureCanvas(self._figure)
        self._canvas.setParent(self)
        self._canvas.setFocusPolicy(Qt.StrongFocus)
        self._canvas.setFocus()

        vbox = QVBoxLayout()
        vbox.addWidget(self._canvas)
        self._toolbar = CustomNavigationToolbar(self._canvas, self)
        self._toolbar.customizationTriggered.connect(self.customizationTriggered)
        vbox.addWidget(self._toolbar)
        self.setLayout(vbox)

        self._dirty = True
        self._active = False
        self.resetPlot()


    def getFigure(self):
        """ :rtype: matplotlib.figure.Figure"""
        return self._figure


    def resetPlot(self):
        self._figure.clear()

    @property
    def name(self):
        """ @rtype: str """
        return self._name

    def updatePlot(self):
        if self.isDirty() and self.isActive():
            # print("Drawing: %s" % self._name)
            self.resetPlot()
            plot_context = self._plotContextFunction(self.getFigure())
            try:
                self._plotFunction(plot_context)
                self._canvas.draw()
            except StandardError as e:
                exc_type, exc_value, exc_tb = sys.exc_info()
                sys.stderr.write("%s\n" % ("-" * 80))
                traceback.print_tb(exc_tb)
                sys.stderr.write("Exception type: %s\n" % exc_type.__name__)
                sys.stderr.write("%s\n" % e.message)
                sys.stderr.write("%s\n" % ("-" * 80))
                sys.stderr.write("An error occurred during plotting. This stack trace is helpful for diagnosing the problem.")

            self.setDirty(False)


    def setDirty(self, dirty=True):
        self._dirty = dirty

    def isDirty(self):
        return self._dirty

    def setActive(self, active=True):
        self._active = active

    def isActive(self):
        return self._active

    def canPlotKey(self, key):
        return any([plotConditionFunction(key) for plotConditionFunction in self._plot_conditions])
