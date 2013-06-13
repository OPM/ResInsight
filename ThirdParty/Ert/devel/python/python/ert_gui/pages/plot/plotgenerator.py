#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'plotgenerator.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import os
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from PyQt4.QtGui import QFrame, QSizePolicy, QVBoxLayout, QCursor, QDialog, QDialogButtonBox
from plotfigure import PlotFigure
from PyQt4.QtCore import Qt, QPoint, QSize, SIGNAL
from PyQt4.QtGui import QProgressBar, QApplication
import threading
from plotsettingsxml import PlotSettingsLoader
from plotsettings import PlotSettings
from plotdata import PlotContextDataFetcher, PlotDataFetcher
from ert_gui.pages.config.parameters.parametermodels import SummaryModel, KeywordModel
import ert.ert.enums as enums

class PlotGenerator(QFrame):

    def __init__(self, plot_path, plot_config_path):
        QFrame.__init__(self)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        self.plot_figure = PlotFigure()
        self.canvas = FigureCanvas(self.plot_figure.getFigure())
        self.canvas.setParent(self)
        self.canvas.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Minimum)

        size = QSize(297*2, 210*2) # A4 aspectratio
        self.canvas.setMaximumSize(size)
        self.canvas.setMinimumSize(size)
        self.setMaximumSize(size)
        self.setMinimumSize(size)

        self.popup = Popup(self)
        self.plot_config_loader = PlotSettingsLoader()
        self.plot_settings = PlotSettings()
        self.plot_settings.setPlotPath(plot_path)
        self.plot_settings.setPlotConfigPath(plot_config_path)
        self.connect(self.popup, SIGNAL('updateProgress(int)'), self.updateProgress)


        self.plot_context_data_fetcher = PlotContextDataFetcher()
        self.plot_context_data_fetcher.initialize(self.plot_context_data_fetcher.getModel())
        self.plot_data_fetcher = PlotDataFetcher()
        self.plot_data_fetcher.initialize(self.plot_data_fetcher.getModel())

    def updateProgress(self, progress = 1):
        value = self.popup.progress_bar.value()
        self.popup.progress_bar.setValue(value + progress)

    def saveAll(self):
        self.popup.show()

        context_data = self.plot_context_data_fetcher.getFromModel()

        save_list = []
        count = 0
        for parameter in context_data.parameters:
            pt = parameter.type

            if pt == SummaryModel.TYPE or pt == KeywordModel.TYPE or pt == enums.obs_impl_type.FIELD_OBS:
                save_list.append(parameter)
                parameter.setUserData({'state' : enums.ert_state_enum.FORECAST})

                if pt == KeywordModel.TYPE:
                    choices = context_data.key_index_list[parameter.name]
                    parameter.getUserData()['key_index_choices'] = choices
                    count += len(choices)
                else:
                    count += 1


        self.popup.progress_bar.setMaximum(count)

        for parameter in save_list:
            if parameter.type == KeywordModel.TYPE:
                for choice in parameter.getUserData()['key_index_choices']:
                    self.plot_data_fetcher.setParameter(parameter, context_data)
                    parameter.getUserData()['key_index'] = choice # because setParameter overwrites this value
                    self.plot_data_fetcher.fetchContent()
                    self.savePlot(self.plot_data_fetcher.data)
            else:
                self.plot_data_fetcher.setParameter(parameter, context_data)
                self.plot_data_fetcher.fetchContent()
                self.savePlot(self.plot_data_fetcher.data)

        self.popup.ok_button.setEnabled(True)

    def save(self, plot_data):
        self.popup.show()

        self.popup.progress_bar.setMaximum(1)

        self.savePlot(plot_data)
        self.popup.ok_button.setEnabled(True)


    def savePlot(self, plot_data):
        generated_plot = self.generatePlot(plot_data)
        if generated_plot:
            self.savePlotToFile(plot_data.getSaveName())
        self.popup.emit(SIGNAL('updateProgress(int)'), 1)

    def generatePlot(self, plot_data):
        name = plot_data.getSaveName()
        load_success = self.plot_config_loader.load(name, self.plot_settings)

        if load_success:
            self.plot_figure.drawPlot(plot_data, self.plot_settings)
            self.canvas.draw()

        return load_success

    def savePlotToFile(self, filename):
        """Save the plot visible in the figure."""
        plot_path = self.plot_settings.getPlotPath()
        if not os.path.exists(plot_path):
            os.makedirs(plot_path)

        path = plot_path + "/" + filename
        self.plot_figure.getFigure().savefig(path + ".png", dpi=400, format="png")
        self.plot_figure.getFigure().savefig(path + ".pdf", dpi=400, format="pdf")


class Popup(QDialog):
    def __init__(self, widget, parent = None):
        QDialog.__init__(self, parent)
        self.setModal(True)
        self.setWindowTitle("Plot save progress")

        layout = QVBoxLayout()
        layout.addWidget(widget)

        self.progress_bar = QProgressBar()
        self.progress_bar.setMinimum(0)
        self.progress_bar.setMaximum(1)
        self.progress_bar.setValue(0)
        layout.addWidget(self.progress_bar)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok, Qt.Horizontal, self)
        layout.addWidget(buttons)
        
        self.setLayout(layout)

        self.connect(buttons, SIGNAL('accepted()'), self.accept)

        self.ok_button = buttons.button(QDialogButtonBox.Ok)
        self.ok_button.setEnabled(False)

    def closeEvent(self, event):
        """Ignore clicking of the x in the top right corner"""
        event.ignore()

    def keyPressEvent(self, event):
        """Ignore ESC keystrokes"""
        if not event.key() == Qt.Key_Escape:
            QDialog.keyPressEvent(self, event)

