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
import ctypes
from types import StringType, IntType
import time
from ert.cwrap import BaseCClass, CWrapper

from ert.job_queue import JOB_QUEUE_LIB, Job


class JobList:
    def __init__(self):
        self.job_list = []
        self.job_dict = {}


    def __getitem__(self, index):
        job = None
        if isinstance(index, StringType):
            job = self.job_dict.get(index)
        elif isinstance(index, IntType):
            try:
                job = self.job_list[index]
            except LookupError:
                job = None
        return job


    def add_job( self, job, job_name ):
        job_index = len(self.job_list)
        job.job_nr = job_index
        self.job_dict[job_name] = job
        self.job_list.append(job)


    @property
    def size(self):
        return len(self.job_list)


class exList:
    def __init__(self, job_list):
        self.job_list = job_list

    def __getitem__(self, index):
        job = self.job_list.__getitem__(index)
        if job:
            return True
        else:
            return False


class statusList:
    def __init__(self, job_list ):
        self.job_list = job_list

    def __getitem__(self, index):
        job = self.job_list.__getitem__(index)
        if job:
            return job.status()
        else:
            return None


class runtimeList:
    def __init__(self, job_list, queue):
        """
        @type job_list: JobList
        @type queue: JobQueue
        """
        self.job_list = job_list
        self.queue = queue

    def __getitem__(self, index):
        job = self.job_list.__getitem__(index)
        if job:
            sim_start = self.queue.cNamespace().iget_sim_start(self.queue, job.job_nr)
            if not sim_start.ctime() == -1:
                return time.time() - sim_start.ctime()
            else:
                return None
        else:
            return None


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

    def __init__(self, driver=None, max_submit=1, size=0):
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
        exit_file = None

        c_ptr = JobQueue.cNamespace().alloc(max_submit, OK_file, exit_file)
        super(JobQueue, self).__init__(c_ptr)

        self.jobs = JobList()
        self.size = size

        self.exists = exList(self.jobs)
        self.status = statusList(self.jobs)
        self.run_time = runtimeList(self.jobs, self)

        self.start(blocking=False)
        if driver:
            self.driver = driver
            JobQueue.cNamespace().set_driver(self, driver.c_ptr)


    def kill_job(self, index):
        """
        Will kill job nr @index.
        """
        job = self.jobs.__getitem__(index)
        if job:
            job.kill()

    def start( self, blocking=False):
        verbose = False
        JobQueue.cNamespace().run_jobs(self, self.size, verbose)


    def submit( self, cmd, run_path, job_name, argv, num_cpu=1):
        c_argv = (ctypes.c_char_p * len(argv))()
        c_argv[:] = argv
        job_index = self.jobs.size

        done_callback = None
        callback_arg = None
        retry_callback = None

        queue_index = JobQueue.cNamespace().add_job_mt(self,
                                                    cmd,
                                                    done_callback,
                                                    retry_callback,
                                                    callback_arg,
                                                    num_cpu,
                                                    run_path,
                                                    job_name,
                                                    len(argv),
                                                    c_argv)

        job = Job(self.driver, JobQueue.cNamespace().get_job_ptr(self, queue_index), queue_index, False)

        self.jobs.add_job(job, job_name)
        return job


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
        JobQueue.cNamespace().submit_complete(self)


    def isRunning(self):
        return JobQueue.cNamespace().is_running(self)

    def num_running( self ):
        return JobQueue.cNamespace().num_running(self)

    def num_pending( self ):
        return JobQueue.cNamespace().num_pending(self)

    def num_waiting( self ):
        return JobQueue.cNamespace().num_waiting(self)

    def num_complete( self ):
        return JobQueue.cNamespace().num_complete(self)

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

    def user_exit(self):
        JobQueue.cNamespace().user_exit(self)

    def set_pause_on(self):
        JobQueue.cNamespace().set_pause_on(self)

    def set_pause_off(self):
        JobQueue.cNamespace().set_pause_off(self)

    def free(self):
        JobQueue.cNamespace().free(self)

#################################################################

cwrapper = CWrapper(JOB_QUEUE_LIB)
cwrapper.registerType("job_queue", JobQueue)
cwrapper.registerType("job_queue_obj", JobQueue.createPythonObject)
cwrapper.registerType("job_queue_ref", JobQueue.createCReference)

JobQueue.cNamespace().alloc           = cwrapper.prototype("c_void_p job_queue_alloc( int , char* , char* )")
JobQueue.cNamespace().user_exit       = cwrapper.prototype("void job_queue_user_exit( job_queue )")
JobQueue.cNamespace().free            = cwrapper.prototype("void job_queue_free( job_queue )")
JobQueue.cNamespace().set_max_running = cwrapper.prototype("void job_queue_set_max_running( job_queue , int)")
JobQueue.cNamespace().get_max_running = cwrapper.prototype("int  job_queue_get_max_running( job_queue )")
JobQueue.cNamespace().set_driver      = cwrapper.prototype("void job_queue_set_driver( job_queue , c_void_p )")
JobQueue.cNamespace().add_job_mt     = cwrapper.prototype("int   job_queue_add_job_mt( job_queue , char* , c_void_p , c_void_p , c_void_p , int , char* , char* , int , char**)")
JobQueue.cNamespace().add_job_st     = cwrapper.prototype("int   job_queue_add_job_st( job_queue , char* , c_void_p , c_void_p , c_void_p , int , char* , char* , int , char**)")
JobQueue.cNamespace().start_queue     = cwrapper.prototype("void job_queue_run_jobs( job_queue , int , bool)")
JobQueue.cNamespace().run_jobs        = cwrapper.prototype("void job_queue_run_jobs_threaded(job_queue , int , bool)")

JobQueue.cNamespace().num_running     = cwrapper.prototype("int  job_queue_get_num_running( job_queue )")
JobQueue.cNamespace().num_complete    = cwrapper.prototype("int  job_queue_get_num_complete( job_queue )")
JobQueue.cNamespace().num_waiting     = cwrapper.prototype("int  job_queue_get_num_waiting( job_queue )")
JobQueue.cNamespace().num_pending     = cwrapper.prototype("int  job_queue_get_num_pending( job_queue )")

JobQueue.cNamespace().is_running      = cwrapper.prototype("int  job_queue_is_running( job_queue )")
JobQueue.cNamespace().submit_complete = cwrapper.prototype("void job_queue_submit_complete( job_queue )")
JobQueue.cNamespace().get_job_ptr     = cwrapper.prototype("c_void_p job_queue_iget_job( job_queue , int)") #warn fix return type
JobQueue.cNamespace().iget_sim_start  = cwrapper.prototype("int job_queue_iget_sim_start( job_queue , int)")
JobQueue.cNamespace().get_active_size = cwrapper.prototype("int job_queue_get_active_size( job_queue )")
JobQueue.cNamespace().get_pause       = cwrapper.prototype("bool job_queue_get_pause(job_queue)")
JobQueue.cNamespace().set_pause_on    = cwrapper.prototype("void job_queue_set_pause_on(job_queue)")
JobQueue.cNamespace().set_pause_off   = cwrapper.prototype("void job_queue_set_pause_off(job_queue)")
