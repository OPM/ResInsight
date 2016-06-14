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
from ert.cwrap import BaseCClass, CWrapper
from ert.job_queue import QueuePrototype, Job, JobStatusType

class JobQueueManager(BaseCClass):
    TYPE_NAME = "job_queue_manager"
    _alloc           = QueuePrototype("void* job_queue_manager_alloc( job_queue)", bind = False)
    _free            = QueuePrototype("void job_queue_manager_free( job_queue_manager )")
    _start_queue     = QueuePrototype("void job_queue_manager_start_queue( job_queue_manager , int , bool, bool)")
    _get_num_waiting = QueuePrototype("int job_queue_manager_get_num_waiting( job_queue_manager )")
    _get_num_running = QueuePrototype("int job_queue_manager_get_num_running( job_queue_manager )")
    _get_num_success = QueuePrototype("int job_queue_manager_get_num_success( job_queue_manager )")
    _get_num_failed  = QueuePrototype("int job_queue_manager_get_num_failed( job_queue_manager )")
    _is_running      = QueuePrototype("bool job_queue_manager_is_running( job_queue_manager )")
    _job_complete    = QueuePrototype("bool job_queue_manager_job_complete( job_queue_manager , int)")
    _job_running     = QueuePrototype("bool job_queue_manager_job_running( job_queue_manager , int)")
    _job_failed      = QueuePrototype("bool job_queue_manager_job_failed( job_queue_manager , int)")
    _job_waiting     = QueuePrototype("bool job_queue_manager_job_waiting( job_queue_manager , int)")
    _job_success     = QueuePrototype("bool job_queue_manager_job_success( job_queue_manager , int)")

    # The return type of the job_queue_manager_iget_job_status should
    # really be the enum job_status_type_enum, but I just did not
    # manage to get the prototyping right. Have therefor taken the
    # return as an integer and convert it in the getJobStatus()
    # method.
    _job_status      = QueuePrototype("int job_queue_manager_iget_job_status(job_queue_manager, int)")

    def __init__(self, queue):
        c_ptr = self._alloc(queue)
        super(JobQueueManager, self).__init__(c_ptr)


    def startQueue(self , total_size , verbose = False , reset_queue = True):
        self._start_queue( total_size , verbose , reset_queue)

    def getNumRunning(self):
        return self._get_num_running(  )

    def getNumWaiting(self):
        return self._get_num_waiting( )

    def getNumSuccess(self):
        return self._get_num_success( )

    def getNumFailed(self):
        return self._get_num_failed(  )
    
    def isRunning(self):
        return self._is_running( )

    def free(self):
        self._free( )

    def isJobComplete(self, job_index):
        return self._job_complete( job_index )

    def isJobRunning(self, job_index):
        return self._job_running( job_index )

    def isJobWaiting(self, job_index):
        return self._job_waiting( job_index )

    def didJobFail(self, job_index):
        return self._job_failed( job_index )

    def didJobSucceed(self, job_index):
        return self._job_success( job_index )

    def getJobStatus(self, job_index):
        # See comment about return type in the prototype section at
        # the top of class.
        """ @rtype: ert.job_queue.job_status_type_enum.JobStatusType """
        int_status = self._job_status(job_index)
        return JobStatusType( int_status )
        
