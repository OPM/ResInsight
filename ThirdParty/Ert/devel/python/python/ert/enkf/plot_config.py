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

    def get_path(self):
        """ @rtype: str """
        return PlotConfig.cNamespace().get_path(self)

    def set_path(self, path):
        PlotConfig.cNamespace().set_path(self, path)

    def get_driver(self):
        """ @rtype: str """
        return PlotConfig.cNamespace().get_driver(self)

    def set_driver(self, driver):
        PlotConfig.cNamespace().set_driver(self, driver)

    def get_errorbar_max(self):
        """ @rtype: int """
        return PlotConfig.cNamespace().get_errorbar_max(self)

    def set_errorbar_max(self, max_value):
        PlotConfig.cNamespace().set_errorbar_max(self, max_value)

    def get_width(self):
        """ @rtype: int """
        return PlotConfig.cNamespace().get_width(self)

    def set_width(self, value):
        PlotConfig.cNamespace().set_width(self, value)

    def get_height(self):
        """ @rtype: int """
        return PlotConfig.cNamespace().get_height(self)

    def set_height(self, value):
        PlotConfig.cNamespace().set_height(self, value)

    def get_viewer(self):
        """ @rtype: str """
        return PlotConfig.cNamespace().get_viewer(self)

    def set_viewer(self, value):
        PlotConfig.cNamespace().set_viewer(self, value)

    def get_image_type(self):
        """ @rtype: str """
        return PlotConfig.cNamespace().get_image_type(self)

    def set_image_type(self, value):
        PlotConfig.cNamespace().set_image_type(self, value)

    def free(self):
        PlotConfig.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("plot_config", PlotConfig)
cwrapper.registerType("plot_config_obj", PlotConfig.createPythonObject)
cwrapper.registerType("plot_config_ref", PlotConfig.createCReference)

PlotConfig.cNamespace().free = cwrapper.prototype("void plot_config_free( plot_config )")

PlotConfig.cNamespace().get_path = cwrapper.prototype("char* plot_config_get_path(plot_config)")
PlotConfig.cNamespace().set_path = cwrapper.prototype("void plot_config_set_path(plot_config, char*)")
PlotConfig.cNamespace().get_driver = cwrapper.prototype("char* plot_config_get_driver(plot_config)")
PlotConfig.cNamespace().set_driver = cwrapper.prototype("void plot_config_set_driver(plot_config, char*)")
PlotConfig.cNamespace().get_errorbar_max = cwrapper.prototype("int plot_config_get_errorbar_max(plot_config)")
PlotConfig.cNamespace().set_errorbar_max = cwrapper.prototype("void plot_config_set_errorbar_max(plot_config, int)")
PlotConfig.cNamespace().get_width = cwrapper.prototype("int plot_config_get_width(plot_config)")
PlotConfig.cNamespace().set_width = cwrapper.prototype("void plot_config_set_width(plot_config, int)")
PlotConfig.cNamespace().get_height = cwrapper.prototype("int plot_config_get_height(plot_config)")
PlotConfig.cNamespace().set_height = cwrapper.prototype("void plot_config_set_height(plot_config, int)")
PlotConfig.cNamespace().get_viewer = cwrapper.prototype("char* plot_config_get_viewer(plot_config)")
PlotConfig.cNamespace().set_viewer = cwrapper.prototype("void plot_config_set_viewer(plot_config, char*)")
PlotConfig.cNamespace().get_image_type = cwrapper.prototype("char* plot_config_get_image_type(plot_config)")
PlotConfig.cNamespace().set_image_type = cwrapper.prototype("void plot_config_set_image_type(plot_config, char*)")

