#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'time_map.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from    ert.cwrap.cwrap        import *
from    ert.cwrap.cclass       import CClass
from    ert.util.ctime         import ctime
from    ert.util.tvector       import * 
from    enkf_enum              import *
import  libenkf
from    ert.enkf.libenkf       import *
from    ert.ert.erttypes import time_t

class TimeMap(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )
            
    @property
    def iget_sim_days(self, step):
        return cfunc.iget_sim_days(self,step)
    
    def iget(self,step):
        return cfunc.iget(self,step)

##################################################################
cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "time_map" , TimeMap )

cfunc = CWrapperNameSpace("time_map")

##################################################################
##################################################################

cfunc.free                  = cwrapper.prototype("void time_map_free( time_map )")           
cfunc.iget_sim_days         = cwrapper.prototype("double time_map_iget_sim_days(time_map, int)")
cfunc.iget                  = cwrapper.prototype("int time_map_iget(time_map, int)")
