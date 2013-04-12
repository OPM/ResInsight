#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'plotview.py' is part of ERT - Ensemble based Reservoir Tool. 
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 


from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas

import datetime
import time
from ert.ert.erttypes import time_t

from ert_gui.widgets.util import print_timing
from  plotdata import PlotData
import ert_gui.widgets

from PyQt4.QtCore import SIGNAL
import os

from plotconfig import PlotConfig
from plotfigure import PlotFigure, matplotlib

from PyQt4.QtGui import QFrame, QInputDialog, QSizePolicy
from plotsettingsxml import PlotSettingsSaver, PlotSettingsLoader
from plotsettings import PlotSettings
from plotsettingsxml import PlotSettingsCopyDialog
from plotgenerator import PlotGenerator

class PlotView(QFrame):
    """PlotView presents a matplotlib canvas with interaction possibilities. (picking and tooltip)"""

    def __init__(self):
        """Create a PlotView instance"""
        QFrame.__init__(self)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        # setup some default data values
        self.data = PlotData()
        self.data.x_data_type = "number"
        self.data.setValid(False)

        self.plot_figure = PlotFigure()
        self.plot_settings = PlotSettings()

        self.canvas = FigureCanvas(self.plot_figure.getFigure())
        self.canvas.setParent(self)
        self.canvas.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        self.mouse_handler = MouseHandler(self)

    def toggleMember(self, line):
        gid = int(line.get_gid())
        if gid in self.plot_settings.getSelectedMembers():
            self.plot_settings.unselectMember(gid)
        else:
            self.plot_settings.selectMember(gid)

    @ert_gui.widgets.util.may_take_a_long_time
    def drawPlot(self):
        self.plot_figure.drawPlot(self.data, self.plot_settings)        
        self.canvas.draw()

    def resizeEvent(self, event):
        QFrame.resizeEvent(self, event)
        self.canvas.resize(event.size().width(), event.size().height())

    def loadSettings(self, name):
        if self.data.isValid():
            plot_config_loader = PlotSettingsLoader()
            if not plot_config_loader.load(name, self.plot_settings):
                self.drawPlot()

    def saveSettings(self):
        if self.data.isValid():
            plot_config_saver = PlotSettingsSaver()
            plot_config_saver.save(self.data.getSaveName(), self.plot_settings)

    def setData(self, data):
        self.saveSettings()

        self.data = data

        self.loadSettings(self.data.getSaveName())


    def setXZoomFactors(self, xminf, xmaxf):
        self.plot_settings.setMinXZoom(xminf)
        self.plot_settings.setMaxXZoom(xmaxf)

    def setYZoomFactors(self, yminf, ymaxf):
        self.plot_settings.setMinYZoom(yminf)
        self.plot_settings.setMaxYZoom(ymaxf)

    def save(self):
        self.saveSettings()

        plot_generator = PlotGenerator(self.plot_settings.getPlotPath(), self.plot_settings.getPlotConfigPath())
        plot_generator.save(self.data)

    def saveAll(self):
        self.saveSettings()

        plot_generator = PlotGenerator(self.plot_settings.getPlotPath(), self.plot_settings.getPlotConfigPath())
        plot_generator.saveAll()
        
    def copyPlotSettings(self):
        plot_config_loader = PlotSettingsLoader()
        plot_config_loader.copy(self.plot_settings)

    def setPlotPath(self, plot_path):
        self.plot_settings.setPlotPath(plot_path)

    def setPlotConfigPath(self, path):
        self.plot_settings.setPlotConfigPath(path)

    def _selectedMemberIdentifier(self, artist):
        return artist.get_gid() in self.plot_settings.getSelectedMembers()

    def clearSelection(self):
        selected_lines = self.plot_figure.fig.findobj(self._selectedMemberIdentifier)
        for line in selected_lines:
            self.plot_settings.unselectMember(line.get_gid())


    def displayToolTip(self, event):
        if not self.data is None and not event.xdata is None and not event.ydata is None:
            if self.data.getXDataType() == "time":
                date = matplotlib.dates.num2date(event.xdata)
                self.setToolTip("x: %s y: %04f" % (date.strftime("%d/%m-%Y"), event.ydata))
            else:
                self.setToolTip("x: %04f y: %04f" % (event.xdata, event.ydata))
        else:
            self.setToolTip("")

    def annotate(self, label, x, y, xt=None, yt=None):
        self.plot_settings.addAnnotation(label, x, y, xt, yt)

    def removeAnnotation(self, annotation_artist):
        annotations = self.plot_settings.getAnnotations()
        for annotation in annotations:
            if annotation.getUserData() == annotation_artist:
                self.plot_settings.removeAnnotation(annotation)

    def moveAnnotation(self, annotation_artist, xt, yt):
        annotations = self.plot_settings.getAnnotations()
        for annotation in annotations:
            if annotation.getUserData() == annotation_artist:
                annotation.xt = xt
                annotation.yt = yt

        annotation_artist.xytext = (xt, yt)

    def draw(self):
        self.canvas.draw()

    def setMinYLimit(self, value):
        self.plot_settings.setMinYLimit(value)

    def setMaxYLimit(self, value):
        self.plot_settings.setMaxYLimit(value)

    def setMinXLimit(self, value):
        self.plot_settings.setMinXLimit(value)

    def setMaxXLimit(self, value):
        self.plot_settings.setMaxXLimit(value)

    def getPlotConfigList(self):
        return self.plot_settings.getPlotConfigList()

class MouseHandler:

    def __init__(self, plot_view):
        self.plot_view = plot_view

        fig = plot_view.plot_figure.getFigure()
        fig.canvas.mpl_connect('button_press_event', self.on_press)
        fig.canvas.mpl_connect('button_release_event', self.on_release)
        fig.canvas.mpl_connect('pick_event', self.on_pick)
        fig.canvas.mpl_connect('motion_notify_event', self.motion_notify_event)

        self.button_position = None
        self.artist = None

    def on_press(self, event):
        if event.button == 3 and self.artist is None and not event.xdata is None and not event.ydata is None:
            label, success = QInputDialog.getText(self.plot_view, "New label", "Enter label:")

            if success and not str(label).strip() == "":
                self.plot_view.annotate(str(label), event.xdata, event.ydata)
                self.plot_view.draw()

    def on_release(self, event):
        self.button_position = None
        self.artist = None

    def on_pick(self, event):
        if isinstance(event.artist, matplotlib.lines.Line2D) and event.mouseevent.button == 1:
            self.plot_view.toggleMember(event.artist)
        elif isinstance(event.artist, matplotlib.text.Annotation) and event.mouseevent.button == 1:
            self.artist = event.artist
            self.button_position = (event.mouseevent.x, event.mouseevent.y)
            return True
        elif isinstance(event.artist, matplotlib.text.Annotation) and event.mouseevent.button == 3:
            self.artist = event.artist
            self.plot_view.removeAnnotation(self.artist)
            self.plot_view.draw()

    def motion_notify_event(self, event):
        if self.artist is None:
            self.plot_view.displayToolTip(event)
        elif isinstance(self.artist, matplotlib.text.Annotation):
            if not event.xdata is None and not event.ydata is None:
                self.plot_view.moveAnnotation(self.artist, event.xdata, event.ydata)
                self.plot_view.draw()


