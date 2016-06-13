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
import os.path

from ert.cwrap import BaseCClass, CWrapper
from ert.job_queue import QueuePrototype


class ExtJob(BaseCClass):
    TYPE_NAME                   = "ext_job"
    _alloc                      = QueuePrototype("void* ext_job_alloc(char*, char*, int)", bind = False)
    _fscanf_alloc               = QueuePrototype("void* ext_job_fscanf_alloc(char*, char*, bool, char* , bool)", bind = False)
    _free                       = QueuePrototype("void ext_job_free( ext_job )")
    _get_help_text              = QueuePrototype("char* ext_job_get_help_text(ext_job)")
    _get_name                   = QueuePrototype("char* ext_job_get_name(ext_job)")
    _get_private_args_as_string = QueuePrototype("char* ext_job_get_private_args_as_string(ext_job)")
    _set_private_args_as_string = QueuePrototype("void ext_job_set_private_args_from_string(ext_job, char*)")
    _is_private                 = QueuePrototype("int ext_job_is_private(ext_job)")
    _get_config_file            = QueuePrototype("char* ext_job_get_config_file(ext_job)")
    _set_config_file            = QueuePrototype("void ext_job_set_config_file(ext_job, char*)")
    _get_stdin_file             = QueuePrototype("char* ext_job_get_stdin_file(ext_job)")
    _set_stdin_file             = QueuePrototype("void ext_job_set_stdin_file(ext_job, char*)")
    _get_stdout_file            = QueuePrototype("char* ext_job_get_stdout_file(ext_job)")
    _set_stdout_file            = QueuePrototype("void ext_job_set_stdout_file(ext_job, char*)")
    _get_stderr_file            = QueuePrototype("char* ext_job_get_stderr_file(ext_job)")
    _set_stderr_file            = QueuePrototype("void ext_job_set_stderr_file(ext_job, char*)")
    _get_target_file            = QueuePrototype("char* ext_job_get_target_file(ext_job)")
    _set_target_file            = QueuePrototype("void ext_job_set_target_file(ext_job, char*)")
    _get_executable             = QueuePrototype("char* ext_job_get_executable(ext_job)")
    _set_executable             = QueuePrototype("void ext_job_set_executable(ext_job, char*)")
    _get_max_running            = QueuePrototype("int ext_job_get_max_running(ext_job)")
    _set_max_running            = QueuePrototype("void ext_job_set_max_running(ext_job, int)")
    _get_max_running_minutes    = QueuePrototype("int ext_job_get_max_running_minutes(ext_job)")
    _set_max_running_minutes    = QueuePrototype("void ext_job_set_max_running_minutes(ext_job, int)")
    _get_environment            = QueuePrototype("void* ext_job_get_environment(ext_job)")
    _set_environment            = QueuePrototype("void ext_job_add_environment(ext_job, char*, char*)")
    _clear_environment          = QueuePrototype("void ext_job_clear_environment(ext_job)")
    _save                       = QueuePrototype("void ext_job_save(ext_job)")


    def __init__(self, config_file, private, name = None , license_root_path = None , search_PATH = True):
        if os.path.isfile( config_file ):
            if name is None:
                name = os.path.basename( config_file )

            c_ptr = self._fscanf_alloc(name, license_root_path, private, config_file , search_PATH)
            super(ExtJob, self).__init__(c_ptr)
        else:
            raise IOError("No such file:%s" % config_file)


    def get_private_args_as_string(self):
        return self._get_private_args_as_string( )

    def set_private_args_as_string(self, args):
        self._set_private_args_as_string( args)

    def get_help_text(self):
        return self._get_help_text( )

    def is_private(self):
        return self._is_private( )

    def get_config_file(self):
        return self._get_config_file( )

    def set_config_file(self, config_file):
        self._set_config_file( config_file)

    def get_stdin_file(self):
        return self._get_stdin_file( )

    def set_stdin_file(self, filename):
        self._set_stdin_file(  filename)

    def get_stdout_file(self):
        return self._get_stdout_file( )

    def set_stdout_file(self, filename):
        self._set_stdout_file( filename)

    def get_stderr_file(self):
        return self._get_stderr_file( )

    def set_stderr_file(self, filename):
        self._set_stderr_file( filename)

    def get_target_file(self):
        return self._get_target_file( )

    def set_target_file(self, filename):
        self._set_target_file( filename)

    def get_executable(self):
        return self._get_executable( )

    def set_executable(self, executable):
        self._set_executable(  executable)

    def get_max_running(self):
        return self._get_max_running( )

    def set_max_running(self, max_running):
        self._set_max_running( max_running)

    def get_max_running_minutes(self):
        return self._get_max_running_minutes( )

    def set_max_running_minutes(self, min_value):
        self._set_max_running_minutes(min_value)

    def get_environment(self):
        return self._get_environment( ) #warn: fix return type

    def set_environment(self, key, value):
        self._set_environment( key, value)

    def clear_environment(self):
        self._clear_environment( )

    def save(self):
        self._save( )

    def free(self):
        self._free( )

    def name(self):
        return self._get_name( )
        


