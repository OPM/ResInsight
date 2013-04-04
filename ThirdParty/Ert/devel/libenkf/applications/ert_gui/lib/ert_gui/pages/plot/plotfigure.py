#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'plotfigure.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from matplotlib.figure import Figure
import matplotlib.lines
import matplotlib.text

import numpy
import ert.ert.erttypes as erttypes
import plotsettings
from plotrenderer import DefaultPlotRenderer

class PlotFigure:
    """A simple wrapper for a matplotlib Figure. Associated with a specified renderer."""
    def __init__(self):
        self.fig = Figure(dpi=100)
        self.axes = self.fig.add_subplot(111)
        self.fig.subplots_adjust(left=0.15)
        self.axes.set_xlim()
        self.plot_renderer = DefaultPlotRenderer()

    def getFigure(self):
        """Return the matplotlib figure."""
        return self.fig

    def drawPlot(self, plot_data, plot_settings):
        """Draw a plot based on the specified data and settings."""
        self.axes.cla()

        self.plot_renderer.drawPlot(self.axes, plot_data, plot_settings)

        if plot_data.getXDataType() == "time":
            self.fig.autofmt_xdate()



