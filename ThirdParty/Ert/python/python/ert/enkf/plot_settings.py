#  Copyright (C) 2012  Statoil ASA, Norway.
#
#  The file 'plot_config.py' is part of ERT - Ensemble based Reservoir Tool.
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
from cwrap import BaseCClass
from ert.config import ConfigSettings
from ert.enkf import EnkfPrototype

class PlotSettings(ConfigSettings):
    TYPE_NAME    = "plot_settings"
    _init        = EnkfPrototype("void plot_settings_init(plot_settings)")
    
    def __init__(self):
        super(PlotSettings, self).__init__("PLOT_SETTING")
        self._init( )
        
    def getPath(self):
        """ @rtype: str """
        return self["PATH"]
        
    def setPath(self, path):
        self["PATH"] = path

    
