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
from ert.cwrap import CWrapper, BaseCClass
from ert.job_queue import JOB_QUEUE_LIB, ExtJob
from ert.util import StringList


class ExtJoblist(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def get_jobs(self):
        """ @rtype: Hash """
        jobs = ExtJoblist.cNamespace().get_jobs(self)
        jobs.setParent(self)
        return jobs

    def getAvailableJobNames(self):
        """ @rtype: StringList """
        return ExtJoblist.cNamespace().alloc_list(self).setParent(self)

    def del_job(self, job):
        return ExtJoblist.cNamespace().del_job(self, job)

    def has_job(self, job):
        return ExtJoblist.cNamespace().has_job(self, job)

    def get_job(self, job):
        """ @rtype: ExtJob """
        return ExtJoblist.cNamespace().get_job(self, job).setParent(self)

    def add_job(self, job_name, new_job):
        ExtJoblist.cNamespace().add_job(self, job_name, new_job)

    def free(self):
        ExtJoblist.cNamespace().free(self)

cwrapper = CWrapper(JOB_QUEUE_LIB)
cwrapper.registerType("ext_joblist", ExtJoblist)
cwrapper.registerType("ext_joblist_obj", ExtJoblist.createPythonObject)
cwrapper.registerType("ext_joblist_ref", ExtJoblist.createCReference)

ExtJoblist.cNamespace().free = cwrapper.prototype("void ext_joblist_free( ext_joblist )")
ExtJoblist.cNamespace().alloc_list = cwrapper.prototype("stringlist_ref ext_joblist_alloc_list(ext_joblist)")
ExtJoblist.cNamespace().get_job = cwrapper.prototype("ext_job_ref ext_joblist_get_job(ext_joblist, char*)")
ExtJoblist.cNamespace().del_job = cwrapper.prototype("int ext_joblist_del_job(ext_joblist, char*)")
ExtJoblist.cNamespace().has_job = cwrapper.prototype("int ext_joblist_has_job(ext_joblist, char*)")
ExtJoblist.cNamespace().add_job = cwrapper.prototype("void ext_joblist_add_job(ext_joblist, char*, ext_joblist)")
ExtJoblist.cNamespace().get_jobs = cwrapper.prototype("hash_ref ext_joblist_get_jobs(ext_joblist)")
