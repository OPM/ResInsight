#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'buffer.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import  libutil
from    ert.enkf.libenkf       import *
#from ert.job_queue.ext_joblist import ExtJoblist

class Buffer(CClass):
    
    def __init__(self , size , parent = None):
        c_ptr = cfunc.alloc(size)
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )
            

##################################################################
cwrapper = CWrapper( libutil.lib )
cwrapper.registerType( "buffer" , Buffer )

cfunc = CWrapperNameSpace("buffer")

##################################################################
##################################################################

cfunc.free  = cwrapper.prototype("void buffer_free( buffer )")           
cfunc.alloc = cwrapper.prototype("c_void_p buffer_alloc(int)")    
