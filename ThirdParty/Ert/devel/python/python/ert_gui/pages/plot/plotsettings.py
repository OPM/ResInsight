#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'plotsettings.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from plotconfig import PlotConfig
import matplotlib
from ert.ert.erttypes import time_t
import datetime
from  PyQt4.QtCore import QObject, SIGNAL

class PlotSettings(QObject):
    """
    PlotSettings are the settings used for plotting.
    Such as color, transparency, x and y limits, annotations.
    """

    plot_color = (55/255.0, 126/255.0, 200/255.0) # bluish
    selected_color = (152/255.0, 78/255.0, 163/255.0) # purple
    history_color = (255/255.0, 0/255.0, 0/255.0) # red
    refcase_color = (0/255.0, 200/255.0, 0/255.0) # green
    comparison_color = (199/255.0, 63/255.0, 0/255.0) # orange

    def __init__(self):
        QObject.__init__(self)

        self._xminf = 0.0
        self._xmaxf = 1.0
        self._x_limits = (None, None)

        self._yminf = 0.0
        self._ymaxf = 1.0
        self._y_limits = (None, None)

        self._plot_path = "."
        self._plot_config_path = "."

        self.observation_plot_config = PlotConfig("Observation", color = self.history_color, zorder=10)
        self.refcase_plot_config = PlotConfig("Refcase", visible=False, color = self.refcase_color, zorder=10)
        self.std_plot_config = PlotConfig("Error", linestyle=":", visible=False, color = self.history_color, zorder=10)
        self.plot_config = PlotConfig("Members", color = self.plot_color, alpha=0.125, zorder=2, picker=2)
        self.selected_plot_config = PlotConfig("Selected members", color = self.selected_color, alpha=0.5, zorder=8,
                                               picker=2)
        self.errorbar_plot_config = PlotConfig("Errorbars", visible=False, color = self.history_color, alpha=0.5,
                                               zorder=10)
        self.comparison_plot_config = PlotConfig("Comparison", color = self.comparison_color, alpha=0.125, zorder=1, visible=True, linestyle="-")

        self._plot_configs = [self.plot_config,
                              self.selected_plot_config,
                              self.refcase_plot_config,
                              self.observation_plot_config,
                              self.std_plot_config,
                              self.errorbar_plot_config,
                              self.comparison_plot_config]

        #for pc in self._plot_configs:
        #    self.connect(pc.signal_handler, SIGNAL('plotConfigChanged(PlotConfig)'), self.notify)

        self._plot_config_dict = {}
        for pc in self._plot_configs:
            self._plot_config_dict[pc.name] = pc

        self._selected_members = []

        self._annotations = []

    def notify(self, *args):
        """Tell listeners that the settings has changed. Automatically called by all setters."""
        self.emit(SIGNAL('plotSettingsChanged(PlotSettings)'), self)

    def getPlotConfigList(self):
        """Get a list of PlotConfig instances."""
        return self._plot_configs

    def getPlotConfigDict(self):
        """Get a dictionary of PlotConfig instances."""
        return self._plot_config_dict

    def getLimitsTuple(self):
        """Get the limits as a tuple: (x_min, x_max, y_min, y_max)"""
        return (self._x_limits[0], self._x_limits[1], self._y_limits[0], self._y_limits[1])

    def getZoomTuple(self):
        """Get the zoom factors as a tuple: (x_min, x_max, y_min, y_max)"""
        return (self._xminf, self._xmaxf, self._yminf, self._ymaxf)

    def selectMember(self, member):
        """Set a member as selected."""
        if not member in self._selected_members:
            self._selected_members.append(int(member))
            self.notify()

    def unselectMember(self, member):
        """Set a member as unselected."""
        member = int(member)
        if member in self._selected_members:
            self._selected_members.remove(member)
            self.notify()

    def clearMemberSelection(self):
        """Clear the list of selected members."""
        self._selected_members = []
        self.notify()

    def getSelectedMembers(self):
        """Returns the list of selected members."""
        return self._selected_members

    def setMinYLimit(self, value):
        """Set the lower limit of the Y axis. Can be set to None."""
        if not value == self._y_limits[0]:
            self._y_limits = (value, self._y_limits[1])
            self.notify()

    def getMinYLimit(self, y_min, data_type=""):
        """Return the lower limit of the Y Axis. Returns y_min if the internal value is None."""
        if self._y_limits[0] is None:
            return y_min
        else:
            return self._y_limits[0]

    def setMaxYLimit(self, value):
        """Set the upper limit of the Y axis. Can be set to None."""
        if not value == self._y_limits[1]:
            self._y_limits = (self._y_limits[0], value)
            self.notify()

    def getMaxYLimit(self, y_max, data_type=""):
        """Return the upper limit of the Y Axis. Returns y_max if the internal value is None."""
        if self._y_limits[1] is None:
            return y_max
        else:
            return self._y_limits[1]

    def setMinXLimit(self, value):
        """Set the lower limit of the X axis. Can be set to None."""
        if not value == self._x_limits[0]:
            self._x_limits = (value, self._x_limits[1])
            self.notify()

    def getMinXLimit(self, x_min, data_type):
        """Returns the provided x_min value if the internal x_min value is None. Converts dates to numbers"""
        if self._x_limits[0] is None:
            x_limit = x_min
        else:
            x_limit = self._x_limits[0]

        if not x_limit is None and data_type == "time" and not isinstance(x_limit, time_t):
            x_limit = time_t(long(round(x_limit)))

        return x_limit

    def setMaxXLimit(self, value):
        """Set the upper limit of the X axis. Can be set to None."""
        if not value == self._x_limits[1]:
            self._x_limits = (self._x_limits[0], value)
            self.notify()

    def getMaxXLimit(self, x_max, data_type):
        """Returns the provided x_max value if the internal x_max value is None. Converts dates to numbers"""
        if self._x_limits[1] is None:
            x_limit = x_max
        else:
            x_limit = self._x_limits[1]

        if not x_limit is None and data_type == "time" and not isinstance(x_limit, time_t):
            x_limit = time_t(long(round(x_limit)))

        return x_limit

    def getLimitStates(self):
        """
        Returns a tuple of True/False values.
        (not x_min is None, not x_max is None, not y_min is None, not y_max is None)
        """
        x_min_state = not self._x_limits[0] is None
        x_max_state = not self._x_limits[1] is None
        y_min_state = not self._y_limits[0] is None
        y_max_state = not self._y_limits[1] is None
        return (x_min_state, x_max_state, y_min_state, y_max_state)

    def setPlotPath(self, plot_path):
        """Set the path to store plot images in."""
        if not plot_path == self._plot_path:
            self._plot_path = plot_path
            self.notify()

    def setPlotConfigPath(self, plot_config_path):
        """Set the path to store plot settings files in."""
        if not plot_config_path == self._plot_config_path:
            self._plot_config_path = plot_config_path
            self.notify()

    def getPlotPath(self):
        """The path to store plot images in."""
        return self._plot_path

    def getPlotConfigPath(self):
        """The path to store plot settings in."""
        return self._plot_config_path

    def setMinXZoom(self, value):
        """Set the x_min zoom factor. [0,1]"""
        if not self._xminf == value:
            self._xminf = value
            self.notify()

    def setMaxXZoom(self, value):
        """Set the x_max zoom factor. [0,1]"""
        if not self._xmaxf == value:
            self._xmaxf = value
            self.notify()

    def setMinYZoom(self, value):
        """Set the y_min zoom factor. [0,1]"""
        if not self._yminf == value:
            self._yminf = value
            self.notify()

    def setMaxYZoom(self, value):
        """Set the y_max zoom factor. [0,1]"""
        if not self._ymaxf == value:
            self._ymaxf = value
            self.notify()

    def getMinXZoom(self):
        """Returns the x_min zoom factor."""
        return self._xminf

    def getMaxXZoom(self):
        """Returns the x_max zoom factor."""
        return self._xmaxf

    def getMinYZoom(self):
        """Returns the y_min zoom factor."""
        return self._yminf

    def getMaxYZoom(self):
        """Returns the y_max zoom factor."""
        return self._ymaxf

    def getAnnotations(self):
        """Return a list of annotations."""
        return self._annotations

    def clearAnnotations(self):
        """Remove all annotations."""
        if len(self._annotations) > 0:
            self._annotations = []
            self.notify()

    def addAnnotation(self, label, x, y, xt, yt):
        """
        Add an annotation. x, y, xt, yt are in data coordinates.
        If any axis represents time. Use matplotlib time values.
        (Float days since 2000.01.01, starting from 1)
        Returns the annotation as a PlotAnnotation object.
        """
        annotation = PlotAnnotation(label, x, y, xt, yt)
        self._annotations.append(annotation)
        self.notify()
        return annotation

    def removeAnnotation(self, annotation):
        """Remove an annotation. Expects PlotAnnotation instance."""
        if annotation in self._annotations:
            self._annotations.remove(annotation)
            self.notify()


class PlotAnnotation:
    """An annotation representation."""
    def __init__(self, label, x, y, xt, yt):
        self.label = label
        self.x = x
        self.y = y
        self.xt = xt
        self.yt = yt

    def setUserData(self, user_data):
        """Usually used to store the matplotlib artist corresponding to this annotation."""
        self._user_data = user_data

    def getUserData(self):
        """Usually the matplotlib artist corresponding to this annotation."""
        return self._user_data








