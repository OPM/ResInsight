#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'run_context.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from ert.job_queue import JobQueueManager
from ert.enkf import RunArg, ENKF_LIB
from ert.util import BoolVector, ArgPack, CThreadPool

class RunContext(object):
    def __init__(self , ert_handle , size , run_fs , run_count):
        self.ert_handle = ert_handle
        self.size = size
        self.runner = ert_handle.getEnkfSimulationRunner()
        
        site_config = self.ert_handle.siteConfig()
        self.queue_manager = JobQueueManager( site_config.getJobQueue() )
        self.queue_manager.startQueue( size , verbose = True )

        mask = BoolVector( default_value = True )
        mask[size - 1] = True
        
        self.ert_handle.addDataKW("<ELCO_RUN_COUNT>" , "%s" % run_count)
        self.ert_run_context = self.ert_handle.getRunContextENSEMPLE_EXPERIMENT( run_fs , mask )
        self.thread_pool = CThreadPool( 8 )
        self.submit_func = CThreadPool.lookupCFunction( ENKF_LIB , "enkf_main_isubmit_job__" )

        
    def isRunning(self):
        return self.queue_manager.isRunning()


    def getNumRunning(self):
        return self.queue_manager.getNumRunning()


    def getNumSuccess(self):
        return self.queue_manager.getNumSuccess()

    
    def getNumFailed(self):
        return self.queue_manager.getNumFailed()

    
    def startSimulation(self , iens):
        member_context = self.ert_run_context.iensGet( iens )
        arg = ArgPack( self.ert_handle , member_context )
        self.thread_pool.addTask( self.submit_func , arg )
        

    def realisationSuccess(self, iens):
        run_arg = self.ert_run_context.iensGet( iens )
        queue_index = run_arg.getQueueIndex()
        return self.queue_manager.jobSuccess( queue_index )

    
    def realisationFailed(self, iens):
        run_arg = self.ert_run_context.iensGet( iens )
        queue_index = run_arg.getQueueIndex()
        return self.queue_manager.jobFailed( queue_index )


    # Running is slightly misleading; we have three catogories:
    # Complete, Failed, and "Running". Running in this context is
    # actually the 'the rest', and includes jobs in all sorts of
    # waiting.
    
    def realisationRunning(self, iens):
        run_arg = self.ert_run_context.iensGet( iens )
        if run_arg.isSubmitted():
            queue_index = run_arg.getQueueIndex()
            if self.queue_manager.jobRunning( queue_index ) or self.queue_manager.jobWaiting( queue_index ):
                return True
            else:
                return False
        else:
            return True
        
