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
import  libjob_queue

class ExtJob(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )
            
    @property
    def get_private_args_as_string(self):
        return cfunc.get_private_args_as_string(self)

    def set_private_args_as_string(self, args):
        cfunc.set_private_args_as_string(self, args)    

    @property
    def get_help_text(self):
        return cfunc.get_help_text(self)
    
    @property
    def is_private(self):
        return cfunc.is_private(self)
        
    @property
    def get_config_file(self):
        return cfunc.get_config_file(self)
    
    def set_config_file(self, config_file):
        cfunc.set_config_file
        
    @property
    def get_stdin_file(self):
        return cfunc.get_stdin_file(self)
        
    def set_stdin_file(self, file):
        cfunc.set_stdin_file(self, file)
        
    @property    
    def get_stdout_file(self):
        return cfunc.get_stdout_file(self)

    def set_stdout_file(self, file):
        cfunc.set_stdout_file(self, file)
        
    @property    
    def get_stderr_file(self):
        return cfunc.get_stderr_file(self)

    def set_stderr_file(self, file):
        cfunc.set_stderr_file(self, file)

    @property
    def get_target_file(self):
        return cfunc.get_target_file(self)

    def set_target_file(self, file):
        cfunc.set_target_file(self, file)
        
    @property    
    def get_executable(self):
        return cfunc.get_executable(self)

    def set_executable(self, executable):
        cfunc.set_executable(self, executable)
        
    @property    
    def get_max_running(self):
        return cfunc.get_max_running(self)

    def set_max_running(self, max_running):
        cfunc.set_max_running(self, max_running)
        
    @property    
    def get_max_running_minutes(self):
        return cfunc.get_max_running_minutes(self)

    def set_max_running_minutes(self, min):
        cfunc.set_max_running_minutes(self, min)
        
    @property    
    def get_environment(self):
        return cfunc.get_environment(self)

    def set_environment(self, key, value):
        cfunc.set_environment(self, key, value)

    def clear_environment(self):
        cfunc. clear_environment(self)
    
    def save(self):
        cfunc.save(self)

    @staticmethod
    def alloc(name, root_path, private):
        job = ExtJob(c_ptr = cfunc.alloc(name, root_path, private))
        return job

    @staticmethod
    def fscanf_alloc(name, root_path, private, config_file):
        job = ExtJob(c_ptr = cfunc.fscanf_alloc(name, root_path, private, config_file))
        return job

##################################################################

cwrapper = CWrapper( libjob_queue.lib )
cwrapper.registerType( "ext_job" , ExtJob )

cfunc = CWrapperNameSpace("ext_job")
##################################################################
##################################################################
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
