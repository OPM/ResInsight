#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'site_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from    ert.util.tvector       import * 
from    enkf_enum              import *
import  libenkf
from    ert.enkf.libenkf       import *
from ert.job_queue.ext_joblist import ExtJoblist
from ert.job_queue.job_queue import JobQueue
from ert.util.stringlist import StringList




class SiteConfig(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )
        

    @property
    def get_queue_name(self):
        return cfunc.get_queue_name( self )

    def set_job_queue(self, queue):
        cfunc.set_job_queue( self , queue)

    @property
    def get_lsf_queue(self):
        return cfunc.get_lsf_queue( self )

    def set_lsf_queue(self, queue):
        cfunc.set_lsf_queue( self , queue)

    @property
    def get_max_running_lsf(self):
        return cfunc.get_max_running_lsf( self )

    def set_max_running_lsf(self, max_running):
        cfunc.set_max_running_lsf( self , max_running)

    @property
    def get_lsf_request(self):
        return cfunc.get_lsf_request( self )

    def set_lsf_request(self, lsf_request):
        cfunc.set_lsf_request( self , lsf_request)

    def clear_rsh_host_list(self):
        cfunc.set_rsh_host_list( self )

    @property
    def get_rsh_command(self):
        return cfunc.get_rsh_command( self )

    def set_rsh_command(self, rsh_command):
        cfunc.set_rsh_command( self , rsh_command)

    @property
    def get_max_running_rsh(self):
        return cfunc.get_max_running_rsh( self )

    def set_max_running_rsh(self, max_running):
        cfunc.set_max_running_rsh( self , max_running)

    @property
    def get_max_running_local(self):
        return cfunc.get_max_running_local( self )

    def set_max_running_local(self, max_running):
        cfunc.set_max_running_local( self , max_running)

    @property
    def get_job_script(self):
        return cfunc.get_job_script( self )

    def set_job_script(self, job_script):
        cfunc.set_job_script( self , job_script)

    @property
    def get_env_hash(self):
        return cfunc.get_env_hash( self )

    def setenv(self, var, value):
        cfunc.setenv( self , var, value)

    def clear_env(self):
        cfunc.clear_env( self )

    @property
    def get_path_variables(self):
        return StringList(c_ptr = cfunc.get_path_variables( self ), parent = self)

    @property
    def get_path_values(self):
        return StringList(c_ptr = cfunc.get_path_values( self ), parent = self)

    def clear_pathvar(self):
        cfunc.clear_pathvar( self )

    def update_pathvar(self, pathvar, value):
        cfunc.update_pathvar( self )

    @property     
    def get_installed_jobs(self):
        installed_jobs = ExtJoblist( cfunc.get_installed_jobs( self ), parent = self)
        return installed_jobs

    @property
    def get_max_submit(self):
        return cfunc.get_max_submit( self )

    def set_max_submit(self, max):
        cfunc.set_max_submit( self , max)

    @property
    def get_license_root_path(self):
        return cfunc.get_license_root_path( self )

    def set_license_root_pathmax_submit(self, path):
        cfunc.set_license_root_path( self , path)

    @property
    def queue_is_running(self):
        return cfunc.queue_is_running( self )

    @property
    def get_job_queue(self):
        return JobQueue( c_ptr = cfunc.get_job_queue(self), parent = self)

    @property
    def get_rsh_host_list(self):
        return cfunc.get_rsh_host_list(self)

    def add_rsh_host(self, host, max_running):
        cfunc.add_rsh_host(self, host, max_running)
##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "site_config" , SiteConfig )

cfunc = CWrapperNameSpace("site_config")

##################################################################
##################################################################

cfunc.free                  = cwrapper.prototype("void site_config_free( site_config )")
cfunc.get_queue_name        = cwrapper.prototype("char* site_config_get_queue_name(site_config)")
cfunc.set_job_queue         = cwrapper.prototype("void site_config_set_job_queue(site_config, char*)")
cfunc.get_lsf_queue         = cwrapper.prototype("char* site_config_get_lsf_queue(site_config)")
cfunc.set_lsf_queue         = cwrapper.prototype("void site_config_set_lsf_queue(site_config, char*)")
cfunc.get_max_running_lsf   = cwrapper.prototype("int site_config_get_max_running_lsf(site_config)")
cfunc.set_max_running_lsf   = cwrapper.prototype("void site_config_set_max_running_lsf(site_config, int)")
cfunc.get_lsf_request       = cwrapper.prototype("char* site_config_get_lsf_request(site_config)")
cfunc.set_lsf_request       = cwrapper.prototype("void site_config_set_lsf_request(site_config, char*)")
cfunc.get_rsh_command       = cwrapper.prototype("char* site_config_get_rsh_command(site_config)")
cfunc.set_rsh_command       = cwrapper.prototype("void site_config_set_rsh_command(site_config, char*)")
cfunc.get_max_running_rsh   = cwrapper.prototype("int site_config_get_max_running_rsh(site_config)")
cfunc.set_max_running_rsh   = cwrapper.prototype("void site_config_set_max_running_rsh(site_config, int)")
cfunc.get_rsh_host_list     = cwrapper.prototype("c_void_p site_config_get_rsh_host_list(site_config)")
cfunc.clear_rsh_host_list   = cwrapper.prototype("void site_config_clear_rsh_host_list(site_config)")
cfunc.add_rsh_host          = cwrapper.prototype("void site_config_add_rsh_host(site_config, char*, int)")
cfunc.get_max_running_local = cwrapper.prototype("int site_config_get_max_running_local(site_config)")
cfunc.set_max_running_local = cwrapper.prototype("void site_config_set_max_running_local(site_config, int)")
cfunc.get_installed_jobs    = cwrapper.prototype("c_void_p site_config_get_installed_jobs(site_config)")
cfunc.get_max_submit        = cwrapper.prototype("int site_config_get_max_submit(site_config)")
cfunc.set_max_submit        = cwrapper.prototype("void site_config_set_max_submit(site_config, int)")
cfunc.get_license_root_path = cwrapper.prototype("char* site_config_get_license_root_path(site_config)")
cfunc.set_license_root_path = cwrapper.prototype("void site_config_set_license_root_path(site_config, char*)")
cfunc.get_job_script        = cwrapper.prototype("char* site_config_get_job_script(site_config)")
cfunc.set_job_script        = cwrapper.prototype("void site_config_set_job_script(site_config, char*)")
cfunc.get_env_hash          = cwrapper.prototype("c_void_p site_config_get_env_hash(site_config)")
cfunc.clear_env             = cwrapper.prototype("void site_config_clear_env(site_config)")
cfunc.setenv                = cwrapper.prototype("void site_config_setenv(site_config, char*, char*)")
cfunc.get_path_variables    = cwrapper.prototype("c_void_p site_config_get_path_variables(site_config)")
cfunc.get_path_values       = cwrapper.prototype("c_void_p site_config_get_path_values(site_config)")
cfunc.clear_pathvar         = cwrapper.prototype("void site_config_clear_pathvar(site_config)")
cfunc.update_pathvar        = cwrapper.prototype("void site_config_update_pathvar(site_config, char*, char*)")
cfunc.get_job_queue         = cwrapper.prototype("c_void_p site_config_get_job_queue(site_config)")
cfunc.queue_is_running      = cwrapper.prototype("bool site_config_queue_is_running(site_config)")
