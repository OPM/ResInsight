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
from ert.enkf import RunArg

class RunContext(object):
    def __init__(self , ert_handle , size , init_case):
        self.ert_handle = ert_handle
        self.size = size
        self.init_case = init_case
        self.runner = ert_handle.getEnkfSimulationRunner()
        
        site_config = self.ert_handle.siteConfig()
        self.queue_manager = JobQueueManager( site_config.getJobQueue() )
        self.queue_manager.startQueue( size , verbose = True )

        fs = self.ert_handle.getEnkfFsManager().getCurrentFileSystem()
        mask = BoolVector( default_value = True )
        mask[size - 1] = True
        self.ert_run_context = self.ert_handle.getRunContextENSEMPLE_EXPERIMENT( fs , mask )
        
        
    def getNumRunning(self):
        return 0


    def getNumComplete(self):
        return 0
        
    
    def startSimulation(self , iens):
        self.ert_handle.submitSimulation( self.ert_run_context.get(iens) )
    
