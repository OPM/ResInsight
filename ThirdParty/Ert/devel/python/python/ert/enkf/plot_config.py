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

import  ctypes
from    ert.cwrap.cwrap       import *
from    ert.cwrap.cclass      import CClass
from    ert.util.tvector      import * 
from    enkf_enum             import *
import  libenkf
class PlotConfig(CClass):
    
    def __init__(self , c_ptr = None):
        self.owner = False
        self.c_ptr = c_ptr
        
        
    def __del__(self):
        if self.owner:
            cfunc.free( self )


    def has_key(self , key):
        return cfunc.has_key( self ,key )



##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "plot_config" , PlotConfig )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("plot_config")


cfunc.free                = cwrapper.prototype("void plot_config_free( plot_config )")
cfunc.get_path            = cwrapper.prototype("char* plot_config_get_path(plot_config)")
cfunc.set_path            = cwrapper.prototype("void plot_config_set_path(plot_config, char*)")
cfunc.get_driver          = cwrapper.prototype("char* plot_config_get_driver(plot_config)")
cfunc.set_driver          = cwrapper.prototype("void plot_config_set_driver(plot_config, char*)")
cfunc.get_errorbar_max    = cwrapper.prototype("int plot_config_get_errorbar_max(plot_config)")
cfunc.set_errorbar_max    = cwrapper.prototype("void plot_config_set_errorbar_max(plot_config, int)")
cfunc.get_width           = cwrapper.prototype("int plot_config_get_width(plot_config)")
cfunc.set_width           = cwrapper.prototype("void plot_config_set_width(plot_config, int)")
cfunc.get_height          = cwrapper.prototype("int plot_config_get_height(plot_config)")
cfunc.set_height          = cwrapper.prototype("void plot_config_set_height(plot_config, int)")
cfunc.get_viewer          = cwrapper.prototype("char* plot_config_get_viewer(plot_config)")
cfunc.set_viewer          = cwrapper.prototype("void plot_config_set_viewer(plot_config, char*)")
cfunc.get_image_type      = cwrapper.prototype("char* plot_config_get_image_type(plot_config)")
cfunc.set_image_type      = cwrapper.prototype("void plot_config_set_image_type(plot_config, char*)")
