from PyQt4.QtCore import Qt
from PyQt4.QtGui import QWidget, QVBoxLayout

from matplotlib.figure import Figure
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas, NavigationToolbar2QT as NavigationToolbar

class PlotWidget(QWidget):
    def __init__(self, name, plotFunction, plot_condition_function_list, plotContextFunction, parent=None):
        QWidget.__init__(self, parent)

        self.__name = name
        self.__plotFunction = plotFunction
        self.__plotContextFunction = plotContextFunction
        self.__plot_conditions = plot_condition_function_list
        """:type: list of functions """
        self.__figure = Figure()
        self.__figure.set_tight_layout(True)
        self.__canvas = FigureCanvas(self.__figure)
        self.__canvas.setParent(self)
        self.__canvas.setFocusPolicy(Qt.StrongFocus)
        self.__canvas.setFocus()

        vbox = QVBoxLayout()
        vbox.addWidget(self.__canvas)
        self.__toolbar = NavigationToolbar(self.__canvas, self)
        vbox.addWidget(self.__toolbar)
        self.setLayout(vbox)

        self.__dirty = True
        self.__active = False
        self.resetPlot()



    def getFigure(self):
        """ :rtype: matplotlib.figure.Figure"""
        return self.__figure


    def resetPlot(self):
        self.__figure.clear()


    def updatePlot(self):
        if self.isDirty() and self.isActive():
            print("Drawing: %s" % self.__name)
            self.resetPlot()
            plot_context = self.__plotContextFunction(self.getFigure())
            self.__plotFunction(plot_context)
            self.__canvas.draw()

            self.setDirty(False)


    def setDirty(self, dirty=True):
        self.__dirty = dirty

    def isDirty(self):
        return self.__dirty

    def setActive(self, active=True):
        self.__active = active

    def isActive(self):
        return self.__active

    def canPlotKey(self, key):
        return any([plotConditionFunction(key) for plotConditionFunction in self.__plot_conditions])