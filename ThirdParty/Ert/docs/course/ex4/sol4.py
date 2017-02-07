#!/usr/bin/env python
import sys
import time 
from ert.enkf import EnKFMain, RunArg, NodeId
from ert.enkf.data import EnkfNode
from ert.job_queue import JobQueueManager

ert = EnKFMain( sys.argv[1] )
fs_manager = ert.getEnkfFsManager( )
fs = fs_manager.getCurrentFileSystem( )


# Initialize the realisations. 
for iens in range( ert.getEnsembleSize()):
    realisation = ert.getRealisation( iens )
    realisation.initialize( fs )


# Fetch out the job_queue from the SiteConfig object. In addition we
# create a JobQueueManager objects which wraps the queue. The purpose
# of this manager object is to let the queue run nonblocking in the
# background.
site_config = ert.siteConfig( )
queue_manager = JobQueueManager( site_config.getJobQueue( ) )
queue_manager.startQueue( ert.getEnsembleSize( ) , verbose = False )


# Create list of RunArg instances which hold metadata for one running
# realisation, create the directory where the simulation should run
# and submit the simulation.
path_fmt = "/tmp/run%d"
arg_list = [ RunArg.createEnsembleExperimentRunArg(fs, iens, path_fmt % iens) for iens in range(ert.getEnsembleSize()) ]
for arg in arg_list:
    ert.createRunPath( arg )
    ert.submitSimulation( arg )

    
while True:
    print("Waiting:%d  Running:%d  Complete:%d/%d" % (queue_manager.getNumWaiting( ), queue_manager.getNumRunning( ) , queue_manager.getNumSuccess() , queue_manager.getNumFailed( )))
    if not queue_manager.isRunning( ):
        break

    time.sleep( 5 )

ens_config = ert.ensembleConfig( )
data_config = ens_config["SNAKE_OIL_OPR_DIFF"]
param_config = ens_config["SNAKE_OIL_PARAM"]    
for iens in range(ert.getEnsembleSize( )):
    data_id = NodeId( realization_number = iens,
                      report_step = 199 )
    enkf_node1 = EnkfNode( data_config )
    enkf_node1.load( fs , data_id )
    gen_data = enkf_node1.asGenData( )
    data = gen_data.getData( )

    
    param_id = NodeId( realization_number = iens,
                       report_step = 0 )
    
    enkf_node2 = EnkfNode( param_config )
    enkf_node2.load( fs , param_id )
    gen_kw = enkf_node2.asGenKw( )
    
    print sum(data)
    for v in gen_kw:
        print v

    # Using the __getitem__() of GenData which was implemented
    # previously.
    for d in gen_data:
        print d
