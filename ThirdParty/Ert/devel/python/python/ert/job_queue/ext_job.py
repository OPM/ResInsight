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
from ert.cwrap import BaseCClass, CWrapper
from ert.job_queue import JOB_QUEUE_LIB


class ExtJob(BaseCClass):
    def __init__(self, name, root_path, private, config_file = None):
        if config_file is None:
            c_ptr = ExtJob.cNamespace().alloc(name, root_path, private)
            super(ExtJob, self).__init__(c_ptr)
        else:
            c_ptr = ExtJob.cNamespace().fscanf_alloc(name, root_path, private, config_file)
            super(ExtJob, self).__init__(c_ptr)


    def get_private_args_as_string(self):
        return ExtJob.cNamespace().get_private_args_as_string(self)

    def set_private_args_as_string(self, args):
        ExtJob.cNamespace().set_private_args_as_string(self, args)

    def get_help_text(self):
        return ExtJob.cNamespace().get_help_text(self)

    def is_private(self):
        return ExtJob.cNamespace().is_private(self)

    def get_config_file(self):
        return ExtJob.cNamespace().get_config_file(self)

    def set_config_file(self, config_file):
        ExtJob.cNamespace().set_config_file(self, config_file)

    def get_stdin_file(self):
        return ExtJob.cNamespace().get_stdin_file(self)

    def set_stdin_file(self, filename):
        ExtJob.cNamespace().set_stdin_file(self, filename)

    def get_stdout_file(self):
        return ExtJob.cNamespace().get_stdout_file(self)

    def set_stdout_file(self, filename):
        ExtJob.cNamespace().set_stdout_file(self, filename)

    def get_stderr_file(self):
        return ExtJob.cNamespace().get_stderr_file(self)

    def set_stderr_file(self, filename):
        ExtJob.cNamespace().set_stderr_file(self, filename)

    def get_target_file(self):
        return ExtJob.cNamespace().get_target_file(self)

    def set_target_file(self, filename):
        ExtJob.cNamespace().set_target_file(self, filename)

    def get_executable(self):
        return ExtJob.cNamespace().get_executable(self)

    def set_executable(self, executable):
        ExtJob.cNamespace().set_executable(self, executable)

    def get_max_running(self):
        return ExtJob.cNamespace().get_max_running(self)

    def set_max_running(self, max_running):
        ExtJob.cNamespace().set_max_running(self, max_running)

    def get_max_running_minutes(self):
        return ExtJob.cNamespace().get_max_running_minutes(self)

    def set_max_running_minutes(self, min_value):
        ExtJob.cNamespace().set_max_running_minutes(self, min_value)

    def get_environment(self):
        return ExtJob.cNamespace().get_environment(self) #warn: fix return type

    def set_environment(self, key, value):
        ExtJob.cNamespace().set_environment(self, key, value)

    def clear_environment(self):
        ExtJob.cNamespace().clear_environment(self)

    def save(self):
        ExtJob.cNamespace().save(self)

    def free(self):
        ExtJob.cNamespace().free(self)

##################################################################

cwrapper = CWrapper(JOB_QUEUE_LIB)
cwrapper.registerType("ext_job", ExtJob)
cwrapper.registerType("ext_job_obj", ExtJob.createPythonObject)
cwrapper.registerType("ext_job_ref", ExtJob.createCReference)


ExtJob.cNamespace().alloc                      = cwrapper.prototype("c_void_p ext_job_alloc(char*, char*, int)")
ExtJob.cNamespace().fscanf_alloc               = cwrapper.prototype("c_void_p ext_job_fscanf_alloc(char*, char*, int, char*)")

ExtJob.cNamespace().free                       = cwrapper.prototype("void ext_job_free( ext_job )")
ExtJob.cNamespace().get_help_text              = cwrapper.prototype("char* ext_job_get_help_text(ext_job)")
ExtJob.cNamespace().get_private_args_as_string = cwrapper.prototype("char* ext_job_get_private_args_as_string(ext_job)")
ExtJob.cNamespace().set_private_args_as_string = cwrapper.prototype("void ext_job_set_private_args_from_string(ext_job, char*)")
ExtJob.cNamespace().is_private                 = cwrapper.prototype("int ext_job_is_private(ext_job)")
ExtJob.cNamespace().get_config_file            = cwrapper.prototype("char* ext_job_get_config_file(ext_job)")
ExtJob.cNamespace().set_config_file            = cwrapper.prototype("void ext_job_set_config_file(ext_job, char*)")
ExtJob.cNamespace().get_stdin_file             = cwrapper.prototype("char* ext_job_get_stdin_file(ext_job)")
ExtJob.cNamespace().set_stdin_file             = cwrapper.prototype("void ext_job_set_stdin_file(ext_job, char*)")
ExtJob.cNamespace().get_stdout_file            = cwrapper.prototype("char* ext_job_get_stdout_file(ext_job)")
ExtJob.cNamespace().set_stdout_file            = cwrapper.prototype("void ext_job_set_stdout_file(ext_job, char*)")
ExtJob.cNamespace().get_stderr_file            = cwrapper.prototype("char* ext_job_get_stderr_file(ext_job)")
ExtJob.cNamespace().set_stderr_file            = cwrapper.prototype("void ext_job_set_stderr_file(ext_job, char*)")
ExtJob.cNamespace().get_target_file            = cwrapper.prototype("char* ext_job_get_target_file(ext_job)")
ExtJob.cNamespace().set_target_file            = cwrapper.prototype("void ext_job_set_target_file(ext_job, char*)")
ExtJob.cNamespace().get_executable             = cwrapper.prototype("char* ext_job_get_executable(ext_job)")
ExtJob.cNamespace().set_executable             = cwrapper.prototype("void ext_job_set_executable(ext_job, char*)")
ExtJob.cNamespace().get_max_running            = cwrapper.prototype("int ext_job_get_max_running(ext_job)")
ExtJob.cNamespace().set_max_running            = cwrapper.prototype("void ext_job_set_max_running(ext_job, int)")
ExtJob.cNamespace().get_max_running_minutes    = cwrapper.prototype("int ext_job_get_max_running_minutes(ext_job)")
ExtJob.cNamespace().set_max_running_minutes    = cwrapper.prototype("void ext_job_set_max_running_minutes(ext_job, int)")
ExtJob.cNamespace().get_environment            = cwrapper.prototype("c_void_p ext_job_get_environment(ext_job)")
ExtJob.cNamespace().set_environment            = cwrapper.prototype("void ext_job_add_environment(ext_job, char*, char*)")
ExtJob.cNamespace().clear_environment          = cwrapper.prototype("void ext_job_clear_environment(ext_job)")
ExtJob.cNamespace().save                       = cwrapper.prototype("void ext_job_save(ext_job)")
