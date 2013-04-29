#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'ext_joblist.py' is part of ERT - Ensemble based Reservoir Tool. 
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
#from    enkf_enum             import *
import  libjob_queue
class ExtJoblist(CClass):
    
    def __init__(self , c_ptr = None):
        self.owner = False
        self.c_ptr = c_ptr
        
        
    def __del__(self):
        if self.owner:
            cfunc.free( self )


    def has_key(self , key):
        return cfunc.has_key( self ,key )



##################################################################

cwrapper = CWrapper( libjob_queue.lib )
cwrapper.registerType( "ext_joblist" , ExtJoblist )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("ext_joblist")


cfunc.free                       = cwrapper.prototype("void ext_joblist_free( ext_joblist )")
cfunc.alloc_list                 = cwrapper.prototype("c_void_p ext_joblist_alloc_list(ext_joblist)")
cfunc.get_job                    = cwrapper.prototype("c_void_p ext_joblist_get_job(ext_joblist, char*)")
cfunc.del_job                    = cwrapper.prototype("int ext_joblist_del_job(ext_joblist, char*)")
cfunc.has_job                    = cwrapper.prototype("int ext_joblist_has_job(ext_joblist, char*)")
cfunc.add_job                    = cwrapper.prototype("void ext_joblist_add_job(ext_joblist, char*, ext_joblist)")
cfunc.get_jobs                   = cwrapper.prototype("c_void_p ext_joblist_get_jobs(ext_joblist)")
