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
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB


class PlotConfig(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getPath(self):
        """ @rtype: str """
        return PlotConfig.cNamespace().get_path(self)

    def setPath(self, path):
        PlotConfig.cNamespace().set_path(self, path)

    def free(self):
        PlotConfig.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("plot_config", PlotConfig)
cwrapper.registerType("plot_config_obj", PlotConfig.createPythonObject)
cwrapper.registerType("plot_config_ref", PlotConfig.createCReference)

PlotConfig.cNamespace().free = cwrapper.prototype("void plot_config_free( plot_config )")
PlotConfig.cNamespace().get_path = cwrapper.prototype("char* plot_config_get_path(plot_config)")
PlotConfig.cNamespace().set_path = cwrapper.prototype("void plot_config_set_path(plot_config, char*)")

