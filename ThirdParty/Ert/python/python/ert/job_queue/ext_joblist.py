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
from cwrap import BaseCClass
from ert.job_queue import QueuePrototype, ExtJob
from ert.util import StringList


class ExtJoblist(BaseCClass):
    TYPE_NAME = "ext_joblist"
    _free       = QueuePrototype("void ext_joblist_free( ext_joblist )")
    _alloc_list = QueuePrototype("stringlist_ref ext_joblist_alloc_list(ext_joblist)")
    _get_job    = QueuePrototype("ext_job_ref ext_joblist_get_job(ext_joblist, char*)")
    _del_job    = QueuePrototype("int ext_joblist_del_job(ext_joblist, char*)")
    _has_job    = QueuePrototype("int ext_joblist_has_job(ext_joblist, char*)")
    _add_job    = QueuePrototype("void ext_joblist_add_job(ext_joblist, char*, ext_joblist)")
    _get_jobs   = QueuePrototype("hash_ref ext_joblist_get_jobs(ext_joblist)")
    _size       = QueuePrototype("int ext_joblist_get_size(ext_joblist)")

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def get_jobs(self):
        """ @rtype: Hash """
        jobs = self._get_jobs( )
        jobs.setParent(self)
        return jobs

    def __len__(self):
        return self._size( )

    def __contains__(self , job):
        return self._has_job(job)

    def __iter__(self):
        names = self.getAvailableJobNames()
        for job in names:
            yield self[job]


    def __getitem__(self, job):
        if job in self:
            return self._get_job(job).setParent(self)
    
    
    def getAvailableJobNames(self):
        """ @rtype: StringList """
        return self._alloc_list( ).setParent(self)

    def del_job(self, job):
        return self._del_job(job)

    def has_job(self, job):
        return job in self

    def get_job(self, job):
        """ @rtype: ExtJob """
        return self[job]

    def add_job(self, job_name, new_job):
        self._add_job(job_name, new_job)

    def free(self):
        self._free( )


