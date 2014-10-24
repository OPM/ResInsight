#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'job_queue_manager.py' is part of ERT - Ensemble based Reservoir Tool. 
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
"""
Module implementing a queue for managing external jobs.

"""
import ctypes
from types import StringType, IntType
import time
from ert.cwrap import BaseCClass, CWrapper

from ert.job_queue import JOB_QUEUE_LIB, Job, JobStatusType



class JobQueueManager(BaseCClass):

    def __init__(self, queue):
        c_ptr = JobQueueManager.cNamespace().alloc(queue)
        super(JobQueueManager, self).__init__(c_ptr)


    def startQueue(self , total_size , verbose = False):
        JobQueueManager.cNamespace().start_queue( self , total_size , verbose )

    def free(self):
        JobQueueManager.cNamespace().free(self)


#################################################################

cwrapper = CWrapper(JOB_QUEUE_LIB)
cwrapper.registerObjectType("job_queue_manager", JobQueueManager)

JobQueueManager.cNamespace().alloc           = cwrapper.prototype("c_void_p job_queue_manager_alloc( job_queue) ")
JobQueueManager.cNamespace().free            = cwrapper.prototype("void job_queue_manager_free( job_queue_manager )")
JobQueueManager.cNamespace().start_queue     = cwrapper.prototype("void job_queue_manager_start_queue( job_queue_manager , int , bool)")
