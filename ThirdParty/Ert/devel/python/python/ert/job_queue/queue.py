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


import  time
import  threading
import  ctypes
from    ert.cwrap.cwrap       import *
from    ert.cwrap.cclass      import CClass


# Need to import this to ensure that the ctime type is registered
import  ert.util.ctime        
from ert.util.ctime import ctime

import  libjob_queue

from    job  import Job

        

class JobList:
    def __init__(self):
        self.job_list = []
        self.job_dict = {}
        

    def __getitem__(self , index):
        job = None
        if isinstance(index , types.StringType):
            job = self.job_dict.get( index )
        elif isinstance(index , types.IntType):
            try:
                job = self.job_list[index]
            except:
                job = None
        return job


    def add_job( self , job , job_name ):
        job_index  = len( self.job_list )
        job.job_nr = job_index
        self.job_dict[ job_name ] = job
        self.job_list.append( job )
        

    @property
    def size(self):
        return len( self.job_list )



class exList:
    def __init__(self , joblist):
        self.joblist = joblist

    def __getitem__(self , index):
        job = self.joblist.__getitem__(index)
        if job:
            return True
        else:
            return False

        

class statusList:
    def __init__(self , joblist ):
        self.joblist = joblist

    def __getitem__(self , index):
        job = self.joblist.__getitem__(index)
        if job:
            return job.status()
        else:
            return None


class runtimeList:
    def __init__(self , joblist , queue):
        self.joblist = joblist
        self.queue   = queue

    def __getitem__(self , index):
        job = self.joblist.__getitem__(index)
        if job:
            sim_start = cfunc.iget_sim_start( self.queue , job.job_nr )
            if not sim_start.ctime() == -1:
                return time.time( ) - sim_start.ctime()
            else:
                return None
        else:
            return None


class JobQueue(CClass):
    
    # If the queue is created with size == 0 that means that it will
    # just grow as needed; for the queue layer to know when to exit
    # you must call the function submit_complete() when you have no
    # more jobs to submit.
    #
    # If the number of jobs is known in advance you can create the
    # queue with a finite value for size, in that case it is not
    # necessary to explitly inform the queue layer when all jobs have
    # been submitted.

    #def __init__(self , driver = None, max_submit = 1 , size = 0, c_ptr = None):
    #    """
    #    SHort doc...
    #    
    #    
    #    
    #    The @size argument is used to say how many jobs the queue will
    #    run, in total.
#
#          size = 0: That means that you do not tell the queue in
#            advance how many jobs you have. The queue will just run
#            all the jobs you add, but you have to inform the queue in
#            some way that all jobs have been submitted. To achieve
#            this you should call the submit_complete() method when all
#            jobs have been submitted.#
#
#          size > 0: The queue will now exactly how many jobs to run,
#            and will continue until this number of jobs have completed
#            - it is not necessary to call the submit_complete() method
#            in this case.
#        """
#
#        OK_file     = None 
#        exit_file   = None
#        if c_ptr:
#            self.init_cobj( c_ptr , cfunc.free_queue )
#        else:
#            c_ptr = cfunc.stringlist_alloc( )
#            self.init_cobj( c_ptr , cfunc.free_queue )
#            
#        self.jobs   = JobList()
#        self.size   = size
#
#        self.exists   = exList( self.jobs )
#        self.status   = statusList( self.jobs )
#        self.run_time = runtimeList( self.jobs , self )
#
#        self.start( blocking = False )
#        if driver:
#            self.driver = driver
#            cfunc.set_driver( self , driver.c_ptr )
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )



    def kill_job(self , index):
        """
        Will kill job nr @index.
        """
        job = self.jobs.__getitem__( index )
        if job:
            job.kill()

    def start( self , blocking = False):
        verbose = False
        cfunc.run_jobs( self , self.size , verbose )


    def submit( self , cmd , run_path , job_name , argv , num_cpu = 1):
        c_argv = (ctypes.c_char_p * len(argv))()
        c_argv[:] = argv
        job_index = self.jobs.size

        done_callback = None
        callback_arg = None
        retry_callback = None
        
        queue_index = cfunc.cadd_job_mt( self , cmd , done_callback , retry_callback , callback_arg , num_cpu , run_path , job_name , len(argv) , c_argv)
        job = Job( self.driver , cfunc.get_job_ptr( self , queue_index ) , queue_index , False )
        
        self.jobs.add_job( job , job_name )
        return job


    def clear( self ):
        pass

    def block_waiting( self ):
        """
        Will block as long as there are waiting jobs.
        """
        while self.num_waiting > 0:
            time.sleep( 1 )
            
    def block(self):
        """
        Will block as long as there are running jobs.
        """
        while self.running:
            time.sleep( 1 )


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
        cfunc.submit_complete( self )


    @property
    def running(self):
        return cfunc.is_running( self )

    @property
    def num_running( self ):
        return cfunc.num_running( self )

    @property
    def num_pending( self ):
        return cfunc.num_pending( self )

    @property
    def num_waiting( self ):
        return cfunc.num_waiting( self )

    @property
    def num_complete( self ):
        return cfunc.num_complete( self )

    def exists(self , index):
        job = self.__getitem__(index)
        if job:
            return True
        else:
            return False

    def get_max_running( self ):
        return self.driver.get_max_running()
    
    def set_max_running( self , max_running ):
        self.driver.set_max_running( max_running )
    
    def user_exit(self):
        cfunc.user_exit(self)

    def set_pause_on(self):
        cfunc.set_pause_on

    def set_pause_off(self):
        cfunc.set_pause_off

#################################################################

cwrapper = CWrapper( libjob_queue.lib )
cwrapper.registerType( "job_queue" , JobQueue )
cfunc  = CWrapperNameSpace( "JobQueue" )

cfunc.user_exit       = cwrapper.prototype("void job_queue_user_exit( job_queue )") 
cfunc.free_queue      = cwrapper.prototype("void job_queue_free( job_queue )")
cfunc.set_max_running = cwrapper.prototype("void job_queue_set_max_running( job_queue , int)")
cfunc.get_max_running = cwrapper.prototype("int  job_queue_get_max_running( job_queue )")
cfunc.set_driver      = cwrapper.prototype("void job_queue_set_driver( job_queue , c_void_p )")
cfunc.cadd_job_mt     = cwrapper.prototype("int  job_queue_add_job_mt( job_queue , char* , c_void_p , c_void_p , c_void_p , int , char* , char* , int , char**)")
cfunc.cadd_job_st     = cwrapper.prototype("int  job_queue_add_job_st( job_queue , char* , c_void_p , c_void_p , c_void_p , int , char* , char* , int , char**)")
cfunc.start_queue     = cwrapper.prototype("void job_queue_run_jobs( job_queue , int , bool)")
cfunc.run_jobs        = cwrapper.prototype("void job_queue_run_jobs_threaded(job_queue , int , bool)")
cfunc.num_running     = cwrapper.prototype("int  job_queue_get_num_running( job_queue )")
cfunc.num_complete    = cwrapper.prototype("int  job_queue_get_num_complete( job_queue )")
cfunc.num_waiting     = cwrapper.prototype("int  job_queue_get_num_waiting( job_queue )")
cfunc.num_pending     = cwrapper.prototype("int  job_queue_get_num_pending( job_queue )")
cfunc.is_running      = cwrapper.prototype("int  job_queue_is_running( job_queue )")
cfunc.submit_complete = cwrapper.prototype("void job_queue_submit_complete( job_queue )")
cfunc.get_job_ptr     = cwrapper.prototype("c_void_p job_queue_iget_job( job_queue , int)")
cfunc.iget_sim_start  = cwrapper.prototype("int   job_queue_iget_sim_start( job_queue , int)")
cfunc.get_active_size = cwrapper.prototype("int      job_queue_get_active_size( job_queue )")
cfunc.get_pause       = cwrapper.prototype("bool job_queue_get_pause(job_queue)")
cfunc.set_pause_on    = cwrapper.prototype("void job_queue_set_pause_on(job_queue)")
cfunc.set_pause_off   = cwrapper.prototype("void job_queue_set_pause_off(job_queue)")
