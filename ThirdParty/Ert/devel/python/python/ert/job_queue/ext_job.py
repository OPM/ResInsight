#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'ext_job.py' is part of ERT - Ensemble based Reservoir Tool. 
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
class ExtJob(CClass):
    
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
cwrapper.registerType( "ext_job" , ExtJob )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("ext_job")


cfunc.free                       = cwrapper.prototype("void ext_job_free( ext_job )")
cfunc.get_help_text              = cwrapper.prototype("char* ext_job_get_help_text(ext_job)")
cfunc.get_private_args_as_string = cwrapper.prototype("char* ext_job_get_private_args_as_string(ext_job)")
cfunc.set_private_args_as_string = cwrapper.prototype("void ext_job_set_private_args_from_string(ext_job, char*)")
cfunc.is_private                 = cwrapper.prototype("int ext_job_is_private(ext_job)")
cfunc.get_config_file            = cwrapper.prototype("char* ext_job_get_config_file(ext_job)")
cfunc.set_config_file            = cwrapper.prototype("void ext_job_set_config_file(ext_job, char*)")
cfunc.alloc                      = cwrapper.prototype("c_void_p ext_job_alloc(char*, char*, int)")
cfunc.fscanf_alloc               = cwrapper.prototype("c_void_p ext_job_fscanf_alloc(char*, char*, int, char*)")
cfunc.get_stdin_file             = cwrapper.prototype("char* ext_job_get_stdin_file(ext_job)")
cfunc.set_stdin_file             = cwrapper.prototype("void ext_job_set_stdin_file(ext_job, char*)")
cfunc.get_stdout_file            = cwrapper.prototype("char* ext_job_get_stdout_file(ext_job)")
cfunc.set_stdout_file            = cwrapper.prototype("void ext_job_set_stdout_file(ext_job, char*)")
cfunc.get_stderr_file            = cwrapper.prototype("char* ext_job_get_stderr_file(ext_job)")
cfunc.set_stderr_file            = cwrapper.prototype("void ext_job_set_stderr_file(ext_job, char*)")
cfunc.get_target_file            = cwrapper.prototype("char* ext_job_get_target_file(ext_job)")
cfunc.set_target_file            = cwrapper.prototype("void ext_job_set_target_file(ext_job, char*)")
cfunc.get_executable             = cwrapper.prototype("char* ext_job_get_executable(ext_job)")
cfunc.set_executable             = cwrapper.prototype("void ext_job_set_executable(ext_job, char*)")
cfunc.get_max_running            = cwrapper.prototype("int ext_job_get_max_running(ext_job)")
cfunc.set_max_running            = cwrapper.prototype("void ext_job_set_max_running(ext_job, int)")
cfunc.get_max_running_minutes    = cwrapper.prototype("int ext_job_get_max_running_minutes(ext_job)")
cfunc.set_max_running_minutes    = cwrapper.prototype("void ext_job_set_max_running_minutes(ext_job, int)")
cfunc.get_environment            = cwrapper.prototype("c_void_p ext_job_get_environment(ext_job)")
cfunc.set_environment            = cwrapper.prototype("void ext_job_add_environment(ext_job, char*, char*)")
cfunc.clear_environment          = cwrapper.prototype("void ext_job_clear_environment(ext_job)")
cfunc.save                       = cwrapper.prototype("void ext_job_save(ext_job)")
