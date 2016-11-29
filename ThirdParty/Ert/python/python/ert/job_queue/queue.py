#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'job_queue.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import sys
from types import StringType, IntType
import time
import ctypes

from cwrap import BaseCClass,BaseCEnum

from ert.job_queue import QueuePrototype
from ert.job_queue import Job, JobStatusType


class JobQueue(BaseCClass):
    # If the queue is created with size == 0 that means that it will
    # just grow as needed; for the queue layer to know when to exit
    # you must call the function submit_complete() when you have no
    # more jobs to submit.
    #
    # If the number of jobs is known in advance you can create the
    # queue with a finite value for size, in that case it is not
    # necessary to explitly inform the queue layer when all jobs have
    # been submitted.
    TYPE_NAME             = "job_queue"
    _alloc                = QueuePrototype("void* job_queue_alloc( int , char* , char* , char* )" , bind      = False)
    _start_user_exit      = QueuePrototype("bool job_queue_start_user_exit( job_queue )")
    _get_user_exit        = QueuePrototype("bool job_queue_get_user_exit( job_queue )")
    _free                 = QueuePrototype("void job_queue_free( job_queue )")
    _set_max_running      = QueuePrototype("void job_queue_set_max_running( job_queue , int)")
    _get_max_running      = QueuePrototype("int  job_queue_get_max_running( job_queue )")
    _set_max_job_duration = QueuePrototype("void job_queue_set_max_job_duration( job_queue , int)")
    _get_max_job_duration = QueuePrototype("int  job_queue_get_max_job_duration( job_queue )")
    _set_driver           = QueuePrototype("void job_queue_set_driver( job_queue , void* )")
    _add_job              = QueuePrototype("int  job_queue_add_job( job_queue , char* , void* , void* , void* , void* , int , char* , char* , int , char**)")
    _kill_job             = QueuePrototype("bool job_queue_kill_job( job_queue , int )")
    _start_queue          = QueuePrototype("void job_queue_run_jobs( job_queue , int , bool)")
    _run_jobs             = QueuePrototype("void job_queue_run_jobs_threaded(job_queue , int , bool)")
    _sim_start            = QueuePrototype("time_t job_queue_iget_sim_start( job_queue , int)")
    _iget_driver_data     = QueuePrototype("void* job_queue_iget_driver_data( job_queue , int)")
    
    _num_running          = QueuePrototype("int  job_queue_get_num_running( job_queue )")
    _num_complete         = QueuePrototype("int  job_queue_get_num_complete( job_queue )")
    _num_waiting          = QueuePrototype("int  job_queue_get_num_waiting( job_queue )")
    _num_pending          = QueuePrototype("int  job_queue_get_num_pending( job_queue )")

    _is_running           = QueuePrototype("bool job_queue_is_running( job_queue )")
    _submit_complete      = QueuePrototype("void job_queue_submit_complete( job_queue )")
    _iget_sim_start       = QueuePrototype("time_t job_queue_iget_sim_start( job_queue , int)")
    _get_active_size      = QueuePrototype("int  job_queue_get_active_size( job_queue )")
    _get_pause            = QueuePrototype("bool job_queue_get_pause(job_queue)")
    _set_pause_on         = QueuePrototype("void job_queue_set_pause_on(job_queue)")
    _set_pause_off        = QueuePrototype("void job_queue_set_pause_off(job_queue)")

    # The return type of the job_queue_iget_job_status should really
    # be the enum job_status_type_enum, but I just did not manage to
    # get the prototyping right. Have therefor taken the return as an
    # integer and convert it in the getJobStatus() method.
    _get_job_status  = QueuePrototype("int job_queue_iget_job_status(job_queue, int)")


    def __init__(self, driver , max_submit=1, size=0):
        """
        Short doc...
        
        The @size argument is used to say how many jobs the queue will
        run, in total.
    
              size = 0: That means that you do not tell the queue in
                advance how many jobs you have. The queue will just run
                all the jobs you add, but you have to inform the queue in
                some way that all jobs have been submitted. To achieve
                this you should call the submit_complete() method when all
                jobs have been submitted.#
    
              size > 0: The queue will know exactly how many jobs to run,
                and will continue until this number of jobs have completed
                - it is not necessary to call the submit_complete() method
                in this case.
            """

        OK_file = None
        status_file = None
        exit_file = None

        c_ptr = self._alloc(max_submit, OK_file, status_file , exit_file)
        super(JobQueue, self).__init__(c_ptr)
        self.size = size

        self.driver = driver
        self._set_driver(driver.from_param(driver))
        self.start( blocking=False )
            

    def kill_job(self, queue_index):
        """
        Will kill job nr @index.
        """
        self._kill_job( queue_index )

        
    def start( self, blocking=False):
        verbose = False
        self._run_jobs(self.size, verbose)


    def submit( self, cmd, run_path, job_name, argv, num_cpu=1):
        c_argv = (ctypes.c_char_p * len(argv))()
        c_argv[:] = argv

        done_callback = None
        callback_arg = None
        retry_callback = None
        exit_callback = None

        queue_index = self._add_job(cmd,
                                    done_callback,
                                    retry_callback,
                                    exit_callback,
                                    callback_arg,
                                    num_cpu,
                                    run_path,
                                    job_name,
                                    len(argv),
                                    c_argv)

        return queue_index


    def clear( self ):
        pass

    def block_waiting( self ):
        """
        Will block as long as there are waiting jobs.
        """
        while self.num_waiting > 0:
            time.sleep(1)

    def block(self):
        """
        Will block as long as there are running jobs.
        """
        while self.isRunning:
            time.sleep(1)


    def submit_complete( self ):
        """
        Method to inform the queue that all jobs have been submitted.

        If the queue has been created with size == 0 the queue has no
        way of knowing when all jobs have completed; hence in that
        case you must call the submit_complete() method when all jobs
        have been submitted.

        If you know in advance exactly how many jobs you will run that
        should be specified with the size argument when creating the
        queue, in that case it is not necessary to call the
        submit_complete() method.
        """
        self._submit_complete( )


    def isRunning(self):
        return self._is_running( )

    def num_running( self ):
        return self._num_running( )

    def num_pending( self ):
        return self._.num_pending( )

    def num_waiting( self ):
        return self._num_waiting( )

    def num_complete( self ):
        return self._num_complete( )

    def exists(self, index):
        job = self.__getitem__(index)
        if job:
            return True
        else:
            return False

    def get_max_running( self ):
        return self.driver.get_max_running()

    def set_max_running( self, max_running ):
        self.driver.set_max_running(max_running)

    def get_max_job_duration(self):
        return self._get_max_job_duration()

    def set_max_job_duration(self, max_duration):
        self._set_max_job_duration(max_duration)

    def killAllJobs(self):
        # The queue will not set the user_exit flag before the
        # queue is in a running state. If the queue does not
        # change to running state within a timeout the C function
        # will return False, and that False value is just passed
        # along.
        user_exit = self._start_user_exit( )
        if user_exit:
            while self.isRunning():
                time.sleep(0.1)
            return True
        else:
            return False

    def igetSimStart(self, job_index):
        return self._iget_sim_start( self , job_index )
        

    def getUserExit(self):
        # Will check if a user_exit has been initated on the job. The
        # queue can be queried about this status until a
        # job_queue_reset() call is invoked, and that should not be
        # done before the queue is recycled to run another batch of
        # simulations.
        return self._get_user_exit( )

    def set_pause_on(self):
        self._set_pause_on( )

    def set_pause_off(self):
        self._set_pause_off( )

    def free(self):
        self._free( )

    def __len__(self):
        return self._get_active_size( )

    def getJobStatus(self, job_number):
        # See comment about return type in the prototype section at
        # the top of class.
        """ @rtype: JobStatusType """
        int_status = self._get_job_status(job_number)
        return JobStatusType( int_status )


