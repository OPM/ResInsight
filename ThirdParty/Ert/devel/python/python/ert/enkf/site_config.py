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
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.job_queue import JobQueue, ExtJoblist
from ert.util import StringList, Hash


class SiteConfig(BaseCClass):
    
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def get_queue_name(self):
        """ @rtype: str """
        return SiteConfig.cNamespace().get_queue_name( self )

    def set_job_queue(self, queue):
        SiteConfig.cNamespace().set_job_queue( self , queue)

    def get_lsf_queue(self):
        """ @rtype: str """
        return SiteConfig.cNamespace().get_lsf_queue( self )

    def set_lsf_queue(self, queue):
        SiteConfig.cNamespace().set_lsf_queue( self , queue)

    def get_max_running_lsf(self):
        """ @rtype: int """
        return SiteConfig.cNamespace().get_max_running_lsf( self )

    def set_max_running_lsf(self, max_running):
        SiteConfig.cNamespace().set_max_running_lsf( self , max_running)

    def get_lsf_request(self):
        """ @rtype: str """
        return SiteConfig.cNamespace().get_lsf_request( self )

    def set_lsf_request(self, lsf_request):
        SiteConfig.cNamespace().set_lsf_request( self , lsf_request)

    def clear_rsh_host_list(self):
        SiteConfig.cNamespace().clear_rsh_host_list( self )

    def get_rsh_command(self):
        """ @rtype: str """
        return SiteConfig.cNamespace().get_rsh_command( self )

    def set_rsh_command(self, rsh_command):
        SiteConfig.cNamespace().set_rsh_command( self , rsh_command)

    def get_max_running_rsh(self):
        """ @rtype: int """
        return SiteConfig.cNamespace().get_max_running_rsh( self )

    def set_max_running_rsh(self, max_running):
        SiteConfig.cNamespace().set_max_running_rsh( self , max_running)

    def get_max_running_local(self):
        """ @rtype: int """
        return SiteConfig.cNamespace().get_max_running_local( self )

    def set_max_running_local(self, max_running):
        SiteConfig.cNamespace().set_max_running_local( self , max_running)

    def get_job_script(self):
        """ @rtype: str """
        return SiteConfig.cNamespace().get_job_script( self )

    def set_job_script(self, job_script):
        SiteConfig.cNamespace().set_job_script( self , job_script)

    def get_env_hash(self):
        """ @rtype: StringHash """
        return SiteConfig.cNamespace().get_env_hash( self )

    def setenv(self, var, value):
        SiteConfig.cNamespace().setenv( self , var, value)

    def clear_env(self):
        SiteConfig.cNamespace().clear_env( self )

    def get_path_variables(self):
        """ @rtype: StringList """
        return SiteConfig.cNamespace().get_path_variables(self).setParent(self)

    def get_path_values(self):
        """ @rtype: StringList """
        return SiteConfig.cNamespace().get_path_values(self).setParent(self)

    def clear_pathvar(self):
        SiteConfig.cNamespace().clear_pathvar( self )

    def update_pathvar(self, pathvar, value):
        SiteConfig.cNamespace().update_pathvar( self, pathvar, value)

    def get_installed_jobs(self):
        """ @rtype: ExtJoblist """
        return SiteConfig.cNamespace().get_installed_jobs(self).setParent(self)

    def get_max_submit(self):
        """ @rtype: int """
        return SiteConfig.cNamespace().get_max_submit( self )

    def set_max_submit(self, max_value):
        SiteConfig.cNamespace().set_max_submit( self , max_value)

    def get_license_root_path(self):
        """ @rtype: str """
        return SiteConfig.cNamespace().get_license_root_path( self )

    def set_license_root_pathmax_submit(self, path):
        SiteConfig.cNamespace().set_license_root_path( self , path)

    def queue_is_running(self):
        """ @rtype: bool """
        return SiteConfig.cNamespace().queue_is_running( self )

    def get_job_queue(self):
        """ @rtype: JobQueue """
        return  SiteConfig.cNamespace().get_job_queue(self).setParent(self)

    def get_rsh_host_list(self):
        """ @rtype: IntegerHash """
        host_list = SiteConfig.cNamespace().get_rsh_host_list(self)
        return host_list

    def add_rsh_host(self, host, max_running):
        SiteConfig.cNamespace().add_rsh_host(self, host, max_running)

    def free(self):
        SiteConfig.cNamespace().free(self)

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType( "site_config" , SiteConfig )
cwrapper.registerType( "site_config_obj" , SiteConfig.createPythonObject)
cwrapper.registerType( "site_config_ref" , SiteConfig.createCReference)


SiteConfig.cNamespace().free                  = cwrapper.prototype("void site_config_free( site_config )")
SiteConfig.cNamespace().get_queue_name        = cwrapper.prototype("char* site_config_get_queue_name(site_config)")
SiteConfig.cNamespace().set_job_queue         = cwrapper.prototype("void site_config_set_job_queue(site_config, char*)")
SiteConfig.cNamespace().get_lsf_queue         = cwrapper.prototype("char* site_config_get_lsf_queue(site_config)")
SiteConfig.cNamespace().set_lsf_queue         = cwrapper.prototype("void site_config_set_lsf_queue(site_config, char*)")
SiteConfig.cNamespace().get_max_running_lsf   = cwrapper.prototype("int site_config_get_max_running_lsf(site_config)")
SiteConfig.cNamespace().set_max_running_lsf   = cwrapper.prototype("void site_config_set_max_running_lsf(site_config, int)")
SiteConfig.cNamespace().get_lsf_request       = cwrapper.prototype("char* site_config_get_lsf_request(site_config)")
SiteConfig.cNamespace().set_lsf_request       = cwrapper.prototype("void site_config_set_lsf_request(site_config, char*)")

SiteConfig.cNamespace().get_rsh_command       = cwrapper.prototype("char* site_config_get_rsh_command(site_config)")
SiteConfig.cNamespace().set_rsh_command       = cwrapper.prototype("void site_config_set_rsh_command(site_config, char*)")
SiteConfig.cNamespace().get_max_running_rsh   = cwrapper.prototype("int site_config_get_max_running_rsh(site_config)")
SiteConfig.cNamespace().set_max_running_rsh   = cwrapper.prototype("void site_config_set_max_running_rsh(site_config, int)")
SiteConfig.cNamespace().get_rsh_host_list     = cwrapper.prototype("integer_hash_ref site_config_get_rsh_host_list(site_config)")
SiteConfig.cNamespace().clear_rsh_host_list   = cwrapper.prototype("void site_config_clear_rsh_host_list(site_config)")
SiteConfig.cNamespace().add_rsh_host          = cwrapper.prototype("void site_config_add_rsh_host(site_config, char*, int)")

SiteConfig.cNamespace().get_max_running_local = cwrapper.prototype("int site_config_get_max_running_local(site_config)")
SiteConfig.cNamespace().set_max_running_local = cwrapper.prototype("void site_config_set_max_running_local(site_config, int)")
SiteConfig.cNamespace().get_installed_jobs    = cwrapper.prototype("ext_joblist_ref site_config_get_installed_jobs(site_config)")
SiteConfig.cNamespace().get_max_submit        = cwrapper.prototype("int site_config_get_max_submit(site_config)")
SiteConfig.cNamespace().set_max_submit        = cwrapper.prototype("void site_config_set_max_submit(site_config, int)")
SiteConfig.cNamespace().get_license_root_path = cwrapper.prototype("char* site_config_get_license_root_path(site_config)")
SiteConfig.cNamespace().set_license_root_path = cwrapper.prototype("void site_config_set_license_root_path(site_config, char*)")
SiteConfig.cNamespace().get_job_script        = cwrapper.prototype("char* site_config_get_job_script(site_config)")
SiteConfig.cNamespace().set_job_script        = cwrapper.prototype("void site_config_set_job_script(site_config, char*)")
SiteConfig.cNamespace().get_env_hash          = cwrapper.prototype("string_hash_ref site_config_get_env_hash(site_config)")
SiteConfig.cNamespace().clear_env             = cwrapper.prototype("void site_config_clear_env(site_config)")
SiteConfig.cNamespace().setenv                = cwrapper.prototype("void site_config_setenv(site_config, char*, char*)")
SiteConfig.cNamespace().get_path_variables    = cwrapper.prototype("stringlist_ref site_config_get_path_variables(site_config)")
SiteConfig.cNamespace().get_path_values       = cwrapper.prototype("stringlist_ref site_config_get_path_values(site_config)")
SiteConfig.cNamespace().clear_pathvar         = cwrapper.prototype("void site_config_clear_pathvar(site_config)")
SiteConfig.cNamespace().update_pathvar        = cwrapper.prototype("void site_config_update_pathvar(site_config, char*, char*)")
SiteConfig.cNamespace().get_job_queue         = cwrapper.prototype("job_queue_ref site_config_get_job_queue(site_config)")
SiteConfig.cNamespace().queue_is_running      = cwrapper.prototype("bool site_config_queue_is_running(site_config)")
