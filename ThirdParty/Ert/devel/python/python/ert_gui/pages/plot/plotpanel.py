#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'plotpanel.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from PyQt4 import QtGui, QtCore
from ert_gui.pages.config.parameters.parameterpanel import Parameter
from ert_gui.pages.plot.plotview import PlotView
import ert_gui.pages.config.parameters.parameterpanel
import ert_gui.widgets.helpedwidget
from ert_gui.widgets.helpedwidget import ContentModel
from ert_gui.pages.config.parameters.parametermodels import DataModel, FieldModel, KeywordModel, SummaryModel
from ert_gui.pages.plot.plotdata import PlotContextDataFetcher, PlotDataFetcher, enums
import ert_gui.widgets.util
import datetime
import time
import matplotlib.dates
from zoomslider import ZoomSlider
from ert_gui.widgets.configpanel import ConfigPanel
from PyQt4.Qt import SIGNAL
from PyQt4.QtCore import QDate, Qt, QPoint , pyqtSignal
from plotconfig import PlotConfigPanel
from PyQt4.QtGui import QTabWidget, QFormLayout, QFrame, QVBoxLayout, QHBoxLayout, QCheckBox, QPushButton, QToolButton, QMainWindow
from PyQt4.QtGui import QCalendarWidget
import plotsettings
import ert.ert.erttypes as erttypes

class PlotPanel(QtGui.QWidget):
    
    changed = pyqtSignal()
    
    def __init__(self):
        QtGui.QWidget.__init__(self)

        plotLayout = QtGui.QHBoxLayout()

        self.plot = PlotView()

        parameterLayout = QtGui.QVBoxLayout()
        self.plotList = QtGui.QListWidget(self)
        self.plotList.setMaximumWidth(150)
        self.plotList.setMinimumWidth(150)

        self.plotDataPanel = PlotParameterConfigurationPanel(self, 150)
        parameterLayout.addWidget(self.plotList)
        parameterLayout.addWidget(self.plotDataPanel)

        self.connect(self.plotList, QtCore.SIGNAL('currentItemChanged(QListWidgetItem *, QListWidgetItem *)'),
                     self.select)
        ContentModel.modelConnect('initialized()', self.updateList)

        #todo: listen to ensemble changes!


        self.plotDataFetcher = PlotDataFetcher()
        self.connect(self.plotDataFetcher, QtCore.SIGNAL('dataChanged()'), self.drawPlot)
        self.plotContextDataFetcher = PlotContextDataFetcher()

        plot_view_layout = QtGui.QGridLayout()

        plot_view_layout.addWidget(self.plot, 0, 0, 1, 1)

        self.h_zoom_slider = ZoomSlider()
        self.connect(self.h_zoom_slider, QtCore.SIGNAL('zoomValueChanged(float, float)'), self.plot.setXZoomFactors)

        self.v_zoom_slider = ZoomSlider(horizontal=False)
        self.connect(self.v_zoom_slider, QtCore.SIGNAL('zoomValueChanged(float, float)'), self.plot.setYZoomFactors)

        plot_view_layout.addWidget(self.h_zoom_slider, 1, 0, 1, 1)
        plot_view_layout.addWidget(self.v_zoom_slider, 0, 1, 1, 1)

        plotLayout.addLayout(parameterLayout)
        plotLayout.addLayout(plot_view_layout)

        self.plotViewSettings = PlotViewSettingsPanel(plotView=self.plot, width=250)
        self.connect(self.plotViewSettings, QtCore.SIGNAL('comparisonCaseSelected(QString)'), self.plotDataFetcher.updateComparisonFS)
        plotLayout.addWidget(self.plotViewSettings)
        
        self.setLayout(plotLayout)

        self.changed.connect( self.fetchSettings )
        #self.connect(self.plot.plot_settings, QtCore.SIGNAL('plotSettingsChanged(PlotSettings)'), self.fetchSettings)
        ContentModel.modelConnect('casesUpdated()', self.updateList)

        
    def drawPlot(self):
        self.plot.setData(self.plotDataFetcher.data)

    def fetchSettings(self, plot_settings):
        if self.plotDataFetcher.data:
            data = self.plotDataFetcher.data
            x_min = plot_settings.getMinXLimit(data.x_min, data.getXDataType())
            x_max = plot_settings.getMaxXLimit(data.x_max, data.getXDataType())
            y_min = plot_settings.getMinYLimit(data.y_min, data.getYDataType())
            y_max = plot_settings.getMaxYLimit(data.y_max, data.getYDataType())

            state = self.h_zoom_slider.blockSignals(True)
            self.h_zoom_slider.setMinValue(plot_settings.getMinXZoom())
            self.h_zoom_slider.setMaxValue(plot_settings.getMaxXZoom())
            self.h_zoom_slider.blockSignals(state)

            state = self.v_zoom_slider.blockSignals(True)
            self.v_zoom_slider.setMinValue(plot_settings.getMinYZoom())
            self.v_zoom_slider.setMaxValue(plot_settings.getMaxYZoom())
            self.v_zoom_slider.blockSignals(state)

            if isinstance(x_min, erttypes.time_t):
                x_min = x_min.value

            if isinstance(x_max, erttypes.time_t):
                x_max = x_max.value

            #todo: time data on y-axis

            state = plot_settings.blockSignals(True)

            self.plotViewSettings.setDataTypes(data.getXDataType(), data.getYDataType())
            self.plotViewSettings.setLimits(x_min, x_max, y_min, y_max)
            self.plotViewSettings.setLimitStates(*plot_settings.getLimitStates())
            self.plotViewSettings.plotSelectionChanged(plot_settings.getSelectedMembers())

            plot_settings.blockSignals(state)

            self.plot.drawPlot()

    @ert_gui.widgets.util.may_take_a_long_time
    def select(self, current, previous):
        if current:
            self.plotDataFetcher.setParameter(current, self.plotContextDataFetcher.data)
            cw = self.plotDataFetcher.getConfigurationWidget(self.plotContextDataFetcher.data)
            self.plotDataPanel.setConfigurationWidget(cw)
            self.plotDataFetcher.fetchContent()
            self.drawPlot()

    def updateList(self):
        self.plotContextDataFetcher.fetchContent()
        self.plotList.clear()
        for parameter in self.plotContextDataFetcher.data.parameters:
            self.plotList.addItem(parameter)

        self.plotList.sortItems()

        self.plot.setPlotPath(self.plotContextDataFetcher.data.plot_path)
        self.plot.setPlotConfigPath(self.plotContextDataFetcher.data.plot_config_path)

        self.plotViewSettings.setCases(self.plotContextDataFetcher.data.getComparableCases())


class PlotViewSettingsPanel(QtGui.QFrame):

    def __init__(self, parent=None, plotView=None, width=100):
        QtGui.QFrame.__init__(self, parent)
        self.setFrameShape(QtGui.QFrame.StyledPanel)
        self.setFrameShadow(QtGui.QFrame.Plain)
        self.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)

        self.setMinimumWidth(width)
        self.setMaximumWidth(width)

        self.plotView = plotView

        layout = QtGui.QVBoxLayout()

        plot_configs = self.plotView.getPlotConfigList()
        tabbed_panel = QTabWidget()
        tabbed_panel.setTabPosition(QTabWidget.West)
        for plot_config in plot_configs:
            config_panel = PlotConfigPanel(plot_config)
            tabbed_panel.addTab(config_panel, plot_config.name)
            self.connect(config_panel, SIGNAL('plotConfigChanged()'), self.plotView.drawPlot)

        layout.addWidget(tabbed_panel)

        tabbed_panel = QTabWidget()
        tabbed_panel.setTabPosition(QTabWidget.West)

        tabbed_panel.addTab(self.createMemberSelectionPanel(), "Members")
        tabbed_panel.addTab(self.createPlotRangePanel(), "Plot")
        tabbed_panel.addTab(self.createButtonLayout(), "Production")
        tabbed_panel.setMaximumHeight(250)

        layout.addWidget(tabbed_panel)

        self.setLayout(layout)

    def setDataTypes(self, x_data_type, y_data_type):
        self.x_min.showDate(x_data_type == "time")
        self.x_max.showDate(x_data_type == "time")
        self.y_min.showDate(y_data_type == "time")
        self.y_max.showDate(y_data_type == "time")


    def setLimits(self, x_min, x_max, y_min, y_max):
        self.x_min.setValue(x_min)
        self.x_max.setValue(x_max)
        self.y_min.setValue(y_min)
        self.y_max.setValue(y_max)

    def setLimitStates(self, x_min_state, x_max_state, y_min_state, y_max_state):
        self.x_min.setChecked(x_min_state)
        self.x_max.setChecked(x_max_state)
        self.y_min.setChecked(y_min_state)
        self.y_max.setChecked(y_max_state)

    def createPlotRangePanel(self):
        frame = QFrame()
        #frame.setMaximumHeight(150)
        #frame.setFrameShape(QFrame.StyledPanel)
        #frame.setFrameShadow(QFrame.Plain)

        layout = QFormLayout()

        self.x_min = DisableableSpinner(self.plotView.setMinXLimit)
        self.x_max = DisableableSpinner(self.plotView.setMaxXLimit)

        layout.addRow("X min:", self.x_min)
        layout.addRow("X max:", self.x_max)

        self.y_min = DisableableSpinner(self.plotView.setMinYLimit)
        self.y_max = DisableableSpinner(self.plotView.setMaxYLimit)

        layout.addRow("Y min:", self.y_min)
        layout.addRow("Y max:", self.y_max)


        layout.addWidget(ert_gui.widgets.util.createSeparator())

        self.plot_compare_to_case = QtGui.QComboBox()
        self.plot_compare_to_case.setToolTip("Select case to compare members against.")


        self.connect(self.plot_compare_to_case, SIGNAL("currentIndexChanged(QString)"), self.select_case)
        layout.addRow("Case:", self.plot_compare_to_case)

        frame.setLayout(layout)
        return frame

    def select_case(self, case):
        self.emit(SIGNAL('comparisonCaseSelected(String)'), str(case))

    def createButtonLayout(self):
        frame = QFrame()
        #frame.setMaximumHeight(150)
        #frame.setFrameShape(QFrame.StyledPanel)
        #frame.setFrameShadow(QFrame.Plain)

        button_layout = QHBoxLayout()

        self.import_button = QtGui.QPushButton()
        self.import_button.setIcon(ert_gui.widgets.util.resourceIcon("plugin"))
        self.import_button.setIconSize(QtCore.QSize(16, 16))
        self.import_button.setToolTip("Copy settings from another plot.")

        self.save_button = QtGui.QPushButton()
        self.save_button.setIcon(ert_gui.widgets.util.resourceIcon("disk"))
        self.save_button.setIconSize(QtCore.QSize(16, 16))
        self.save_button.setToolTip("Save a plot.")

        self.save_many_button = QtGui.QPushButton()
        self.save_many_button.setIcon(ert_gui.widgets.util.resourceIcon("save_plots"))
        self.save_many_button.setIconSize(QtCore.QSize(16, 16))
        self.save_many_button.setToolTip("Save all configured plots.")

        button_layout.addWidget(self.import_button)
        button_layout.addWidget(self.save_button)
        button_layout.addWidget(self.save_many_button)

        self.connect(self.save_button, QtCore.SIGNAL('clicked()'), self.plotView.save)
        self.connect(self.save_many_button, QtCore.SIGNAL('clicked()'), self.plotView.saveAll)
        self.connect(self.import_button, QtCore.SIGNAL('clicked()'), self.plotView.copyPlotSettings)

        vertical_layout = QVBoxLayout()
        vertical_layout.addLayout(button_layout)
        vertical_layout.addStretch(1)
        frame.setLayout(vertical_layout)

        return frame

    def createMemberSelectionPanel(self):
        frame = QFrame()
        #frame.setMinimumHeight(100)
        #frame.setMaximumHeight(100)
        #frame.setFrameShape(QFrame.StyledPanel)
        #frame.setFrameShadow(QFrame.Plain)

        layout = QVBoxLayout()

        self.selected_member_label = QtGui.QLabel()
        self.selected_member_label.setWordWrap(True)

        layout.addWidget(QtGui.QLabel("Selected members:"))
        layout.addWidget(self.selected_member_label)

        layout.addStretch(1)

        self.clear_button = QtGui.QPushButton()
        self.clear_button.setText("Clear selection")
        layout.addWidget(self.clear_button)
        self.connect(self.clear_button, QtCore.SIGNAL('clicked()'), self.plotView.clearSelection)

        layout.addStretch(1)
        frame.setLayout(layout)

        return frame

    def plotSelectionChanged(self, selected_members):
        if isinstance(selected_members, plotsettings.PlotSettings):
            selected_members = selected_members.getSelectedMembers()
        text = ""
        for member in selected_members:
            text = text + " " + str(member)
        self.selected_member_label.setText(text)

    def setCases(self, cases):
        state = self.plot_compare_to_case.blockSignals(True)
        self.plot_compare_to_case.clear()
        self.plot_compare_to_case.addItems(cases)
        self.plot_compare_to_case.blockSignals(state)
        self.select_case("None")

class DisableableSpinner(QFrame):

    def __init__(self, func):
        QFrame.__init__(self)
        self.func = func

        layout = QHBoxLayout()
        layout.setMargin(0)

        self.check = QCheckBox()

        self.spinner = QtGui.QDoubleSpinBox()
        self.spinner.setSingleStep(1)
        self.spinner.setDisabled(True)
        self.spinner.setMinimum(-10000000000)
        self.spinner.setMaximum(10000000000)
        self.spinner.setMaximumWidth(100)
        self.connect(self.spinner, QtCore.SIGNAL('valueChanged(double)'), self.update)

        self.date_spinner = QtGui.QDateEdit()
        self.date_spinner.setDisabled(True)
        self.date_spinner.setDisplayFormat("dd/MM-yyyy")
        self.date_spinner.setMaximumWidth(100)
        self.date_spinner.setCalendarPopup(True)
        self.connect(self.date_spinner, QtCore.SIGNAL('dateChanged(QDate)'), self.update)

        self.connect(self.check, SIGNAL('stateChanged(int)'), self.disabler)

        layout.addWidget(self.check)
        layout.addWidget(self.spinner)
        layout.addWidget(self.date_spinner)


        self.setLayout(layout)

        self.showDate(False)

    def update(self, value):
        if self.show_date:
            seconds = self.qDateToSeconds(value)
            self.func(seconds)
        else:
            self.func(value)

    def disabler(self, state):
        disabled = not state == 2
        self.spinner.setDisabled(disabled)
        self.date_spinner.setDisabled(disabled)

        if not disabled:
            if self.show_date:
                seconds = self.qDateToSeconds(self.date_spinner.date())
                self.func(seconds)
            else:
                self.func(self.spinner.value())
        else:
            self.func(None)

    def setChecked(self, state):
        self.check.setChecked(state)

    def setValue(self, value):
        if not value is None:
            if self.show_date:
                state = self.date_spinner.blockSignals(True)
                self.date_spinner.setDate(self.qDateFromSeconds(value))
                self.date_spinner.blockSignals(state)
            else:
                state = self.spinner.blockSignals(True)
                self.spinner.setValue(value)
                self.spinner.blockSignals(state)

    def showDate(self, bool):
        self.show_date = bool
        self.spinner.setHidden(bool)
        self.date_spinner.setHidden(not bool)

    def qDateFromSeconds(self, seconds):
        t = time.localtime(seconds)
        return QDate(*t[0:3])

    def qDateToSeconds(self, qdate):
        date = qdate.toPyDate()
        seconds = int(time.mktime(date.timetuple()))
        return seconds


class PlotParameterConfigurationPanel(QtGui.QFrame):
    def __init__(self, parent=None, width=100):
        QtGui.QFrame.__init__(self, parent)
        self.setFrameShape(QtGui.QFrame.StyledPanel)
        self.setFrameShadow(QtGui.QFrame.Plain)
        self.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)

        self.setMinimumWidth(width)
        self.setMaximumWidth(width)
        self.setMaximumHeight(200)

        self.layout = QtGui.QStackedLayout()
        self.setLayout(self.layout)

    def setConfigurationWidget(self, widget):
        if self.layout.indexOf(widget) == -1:
            self.layout.addWidget(widget)
        self.layout.setCurrentWidget(widget)



