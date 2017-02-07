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
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.job_queue import JobQueue, ExtJoblist
from ert.util import StringList, Hash


class SiteConfig(BaseCClass):
    TYPE_NAME = "site_config"

    _free                  = EnkfPrototype("void site_config_free( site_config )")
    _get_queue_name        = EnkfPrototype("char* site_config_get_queue_name(site_config)")
    _get_lsf_queue         = EnkfPrototype("char* site_config_get_lsf_queue(site_config)")
    _set_lsf_queue         = EnkfPrototype("void site_config_set_lsf_queue(site_config, char*)")
    _get_max_running_lsf   = EnkfPrototype("int site_config_get_max_running_lsf(site_config)")
    _set_max_running_lsf   = EnkfPrototype("void site_config_set_max_running_lsf(site_config, int)")
    _get_lsf_request       = EnkfPrototype("char* site_config_get_lsf_request(site_config)")
    _set_lsf_request       = EnkfPrototype("void site_config_set_lsf_request(site_config, char*)")
    _get_rsh_command       = EnkfPrototype("char* site_config_get_rsh_command(site_config)")
    _set_rsh_command       = EnkfPrototype("void site_config_set_rsh_command(site_config, char*)")
    _get_max_running_rsh   = EnkfPrototype("int site_config_get_max_running_rsh(site_config)")
    _set_max_running_rsh   = EnkfPrototype("void site_config_set_max_running_rsh(site_config, int)")
    _get_rsh_host_list     = EnkfPrototype("integer_hash_ref site_config_get_rsh_host_list(site_config)")
    _clear_rsh_host_list   = EnkfPrototype("void site_config_clear_rsh_host_list(site_config)")
    _add_rsh_host          = EnkfPrototype("void site_config_add_rsh_host(site_config, char*, int)")
    _get_max_running_local = EnkfPrototype("int site_config_get_max_running_local(site_config)")
    _set_max_running_local = EnkfPrototype("void site_config_set_max_running_local(site_config, int)")
    _get_installed_jobs    = EnkfPrototype("ext_joblist_ref site_config_get_installed_jobs(site_config)")
    _get_max_submit        = EnkfPrototype("int site_config_get_max_submit(site_config)")
    _set_max_submit        = EnkfPrototype("void site_config_set_max_submit(site_config, int)")
    _get_license_root_path = EnkfPrototype("char* site_config_get_license_root_path(site_config)")
    _set_license_root_path = EnkfPrototype("void site_config_set_license_root_path(site_config, char*)")
    _get_job_script        = EnkfPrototype("char* site_config_get_job_script(site_config)")
    _set_job_script        = EnkfPrototype("void site_config_set_job_script(site_config, char*)")
    _get_env_hash          = EnkfPrototype("string_hash_ref site_config_get_env_hash(site_config)")
    _clear_env             = EnkfPrototype("void site_config_clear_env(site_config)")
    _setenv                = EnkfPrototype("void site_config_setenv(site_config, char*, char*)")
    _get_path_variables    = EnkfPrototype("stringlist_ref site_config_get_path_variables(site_config)")
    _get_path_values       = EnkfPrototype("stringlist_ref site_config_get_path_values(site_config)")
    _clear_pathvar         = EnkfPrototype("void site_config_clear_pathvar(site_config)")
    _update_pathvar        = EnkfPrototype("void site_config_update_pathvar(site_config, char*, char*)")
    _get_job_queue         = EnkfPrototype("job_queue_ref site_config_get_job_queue(site_config)")
    _queue_is_running      = EnkfPrototype("bool site_config_queue_is_running(site_config)")
    _get_location          = EnkfPrototype("char* site_config_get_location(site_config)")
    _has_driver            = EnkfPrototype("bool site_config_has_queue_driver(site_config, char*)")


    
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getQueueName(self):
        """ @rtype: str """
        return self._get_queue_name( )

    def setJobQueue(self, queue):
        raise Exception("The function setJobQueue() is not properly implemented")


    def hasDriver(self, driver_name):
        return self._has_driver( driver_name )

    
    def getLsfQueue(self):
        """ @rtype: str """
        return self._get_lsf_queue( )

    def setLsfQueue(self, queue):
        self._set_lsf_queue( queue)

    def getMaxRunningLsf(self):
        """ @rtype: int """
        return self._get_max_running_lsf( )

    def setMaxRunningLsf(self, max_running):
        self._set_max_running_lsf( max_running)

    def getLsfRequest(self):
        """ @rtype: str """
        return self._get_lsf_request( )

    def setLsfRequest(self, lsf_request):
        self._set_lsf_request( lsf_request)

    def clearRshHostList(self):
        self._clear_rsh_host_list( )

    def getRshCommand(self):
        """ @rtype: str """
        return self._get_rsh_command( )

    def set_rsh_command(self, rsh_command):
        self._set_rsh_command( rsh_command)

    def getMaxRunningRsh(self):
        """ @rtype: int """
        return self._get_max_running_rsh(  )

    def setMaxRunningRsh(self, max_running):
        self._set_max_running_rsh(  max_running)

    def getMaxRunningLocal(self):
        """ @rtype: int """
        return self._get_max_running_local( )

    def setMaxRunningLocal(self, max_running):
        self._set_max_running_local(  max_running)

    def get_job_script(self):
        """ @rtype: str """
        return self._get_job_script(  )

    def set_job_script(self, job_script):
        self._set_job_script( job_script)

    def get_env_hash(self):
        """ @rtype: StringHash """
        return self._get_env_hash( )

    def setenv(self, var, value):
        self._setenv( var, value)

    def clear_env(self):
        self._clear_env(  )

    def get_path_variables(self):
        """ @rtype: StringList """
        return self._get_path_variables().setParent(self)

    def get_path_values(self):
        """ @rtype: StringList """
        return self._get_path_values().setParent(self)

    def clear_pathvar(self):
        self._clear_pathvar(  )

    def update_pathvar(self, pathvar, value):
        self._update_pathvar( pathvar, value)

    def get_installed_jobs(self):
        """ @rtype: ExtJoblist """
        return self._get_installed_jobs().setParent(self)

    def get_max_submit(self):
        """ @rtype: int """
        return self._get_max_submit( )

    def set_max_submit(self, max_value):
        self._set_max_submit( max_value)

    def get_license_root_path(self):
        """ @rtype: str """
        return self._get_license_root_path( )

    def set_license_root_pathmax_submit(self, path):
        self._set_license_root_path( path)

    def isQueueRunning(self):
        """ @rtype: bool """
        return self._queue_is_running( )

    def getJobQueue(self):
        """ @rtype: JobQueue """
        return  self._get_job_queue().setParent(self)

    def getRshHostList(self):
        """ @rtype: IntegerHash """
        host_list = self._get_rsh_host_list()
        return host_list

    def addRshHost(self, host, max_running):
        self._add_rsh_host(host, max_running)

    def getLocation(self):
        """ @rtype: str """
        return self._get_location()


    def free(self):
        self._free()


