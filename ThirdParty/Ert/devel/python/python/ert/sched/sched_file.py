#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'sched_file.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.cwrap import CClass, CWrapper, CWrapperNameSpace
from ert.sched import SCHED_LIB
from ert.util import ctime


class SchedFile(CClass):

    def __init__(self , filename , start_time):
        c_ptr = cfunc.parse( filename , ctime( start_time ))
        self.init_cobj( c_ptr , cfunc.free )


    @property
    def length(self):
        return cfunc.length( self )

    def write( self , filename , num_dates , add_end = True):
        cfunc.write(self , num_dates , filename , add_end)



cwrapper = CWrapper(SCHED_LIB)
cwrapper.registerType( "sched_file" , SchedFile )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("sched_file")

cfunc.parse  = cwrapper.prototype("c_void_p sched_file_parse_alloc( char*, time_t )")
cfunc.write  = cwrapper.prototype("void     sched_file_fprintf_i( sched_file , int , char* , bool)")
cfunc.length = cwrapper.prototype("int      sched_file_get_num_restart_files( sched_file )")
cfunc.free   = cwrapper.prototype("void     sched_file_free( sched_file )")
