#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'ert_server.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import logging

import sys
import threading
import json
import os
import traceback
import datetime
from time import strftime

from ert.enkf import EnKFMain,RunArg,EnkfFsManager
from ert.enkf.enums import EnkfRunType, EnkfStateType, ErtImplType , EnkfVarType , RealizationStateEnum
from ert.enkf import NodeId
from ert.util import installAbortSignals

from .run_context import RunContext

# The server will always return SUCCESS(), or alternatively raise an
# exception. The ert_socket layer will catch the exception an return
# an error message to the client. The error message will be created
# with the ERROR function below.


def SUCCESS(res):
    return ["OK"] + res
    

def ERROR(msg , exception = None):
    result = ["ERROR", msg]
    if exception:
        result.append( "%s" % exception )
    return result
            


class ErtServer(object):
    DATE_FORMAT = '%Y-%m-%d %H:%M:%S'
    site_config = None

    def __init__(self , config , logger=None):
        installAbortSignals()

        self.queue_lock = threading.Lock()
        self.ert_handle = None

        if logger is None:
            logger = logging

        self.logger = logger

        self.open(config)

        self.initCmdTable()
        self.run_context = None
        self.init_fs = None
        self.run_fs = None
        self.run_count = 0


    def SUCCESS(self , res):
        self.logger.debug("Success: returning: %s" , res)
        return SUCCESS(res)


    def ERROR(self , res):
        self.logger.debug("ERROR: returning: %s" , res)
        return ERROR(res)


    def initCmdTable(self):
        self.cmd_table = {"STATUS" : self.handleSTATUS ,
                          "INIT_SIMULATIONS" : self.handleINIT_SIMULATIONS ,
                          "ADD_SIMULATION" : self.handleADD_SIMULATION ,
                          "GET_RESULT" : self.handleGET_RESULT ,
                          "TIME_STEP": self.handleTIMESTEP }


    def open(self , config):
        if isinstance(config, EnKFMain):
            config_file = config.getUserConfigFile()
        else:
            if os.path.exists(config):
                config_file = config
                config = EnKFMain( config )
            else:
                raise IOError("The config file:%s does not exist" % config)

        self.config_file = config_file
        self.ert_handle = config
        self.logger.info("Have connect ert handle to:%s" , config_file)


    def close(self):
        # More cleanup first ...
        self.logger.info("Shutting down ert handle")
        self.ert_handle = None


    def isConnected(self):
        if self.ert_handle:
            return True
        else:
            return False


    def __del__(self):
        if self.isConnected():
            self.close()



    def evalCmd(self , cmd_expr):
        cmd = cmd_expr[0]
        func = self.cmd_table.get(cmd)
        self.logger.info("Received command: %s" % cmd)
        
        if func:
            return func(cmd_expr[1:])
        else:
            raise KeyError("The command:%s was not recognized" % cmd)



    # The STATUS action can either report results for the complete
    # simulation set, or it can report the status of one particular
    # realisation. If the function is called with zero arguments it
    # will return the global status, if called with one argument it
    # will return the status for that realisation.
        
    def handleSTATUS(self , args):
        if self.isConnected():
            if self.run_context is None:
                return self.SUCCESS(["READY"])
            else:
                if len(args) == 0:
                    if self.run_context.isRunning():
                        return self.SUCCESS(["RUNNING" , self.run_context.getNumRunning() , self.run_context.getNumSuccess() , self.run_context.getNumFailed()])
                    else:
                        return self.SUCCESS(["COMPLETE" , self.run_context.getNumRunning() , self.run_context.getNumSuccess() , self.run_context.getNumFailed()])
                else:
                    iens = args[0]

                    if self.run_context.realisationRunning(iens):
                        return self.SUCCESS(["RUNNING"])
                    elif self.run_context.realisationFailed(iens):
                        return self.SUCCESS(["FAILED"])
                    elif self.run_context.realisationSuccess(iens):
                        return self.SUCCESS(["SUCCESS"])

        else:
            return self.SUCCESS(["CLOSED"])

    
    def initSimulations(self , args):
        run_size = args[0]
        init_case = str(args[1])
        run_case = str(args[2])
        
        fs_manager = self.ert_handle.getEnkfFsManager()
        self.run_fs = fs_manager.getFileSystem( run_case )
        self.init_fs = fs_manager.getFileSystem( init_case )
        fs_manager.switchFileSystem( self.run_fs )

        self.run_context = RunContext(self.ert_handle , run_size , self.run_fs  , self.run_count)
        self.run_count += 1
        return self.handleSTATUS([])


    def restartSimulations(self , args):
        return self.initSimulations(args)


    def handleINIT_SIMULATIONS(self , args):
        if len(args) == 3:
            result = []
            with self.queue_lock:
                if self.run_context is None:
                    self.initSimulations( args )
                else:
                    if not self.run_context.isRunning():
                        self.restartSimulations( args )
                
                result = ["OK"]
                
            return self.SUCCESS(result)
        else:
            raise IndexError("The INIT_SIMULATIONS command expects three arguments: [ensemble_size , init_case, run_case]")


    
    def handleGET_RESULT(self , args):
        iens = args[0]
        kw = str(args[2])

        try:
            year,month,day = args[1]
            time_map = self.run_fs.getTimeMap( )
            report_step = time_map.lookupTime( datetime.date( year , month , day) , tolerance_seconds_before = 24*3600 , tolerance_seconds_after = 24*3600)
        except TypeError:
            report_step = args[1]

        ensembleConfig = self.ert_handle.ensembleConfig()
        if kw in ensembleConfig:
            state = self.ert_handle.getRealisation( iens )
            node = state[kw]
            gen_data = node.asGenData()
            
            fs = self.ert_handle.getEnkfFsManager().getCurrentFileSystem()
            node_id = NodeId(report_step , iens , EnkfStateType.FORECAST )
            if node.tryLoad( fs , node_id ):
                data = gen_data.getData()
                return self.SUCCESS( ["OK"] + data.asList() )
            else:
                raise Exception("Loading iens:%d  report:%d   kw:%s   failed" % (iens , report_step , kw))
        else:
            raise KeyError("The keyword:%s is not recognized" % kw)





    # ["ADD_SIMULATION" , 0 , 1 , 1 [ ["KW1" , ...] , ["KW2" , ....]]]
    def handleADD_SIMULATION(self , args):
        geo_id = args[0]
        pert_id = args[1]
        iens = args[2]
        self.logger.debug("ADD_SIMULATION  geo_id:%d  pert_id:%d  iens:%d" % (geo_id , pert_id , iens))
        kw_list = args[3]
        state = self.ert_handle.getRealisation( iens )
        state.addSubstKeyword( "GEO_ID" , "%d" % geo_id )
        
        elco_kw = [ l[0] for l in kw_list ]
        ens_config = self.ert_handle.ensembleConfig()

        for kw in ens_config.getKeylistFromVarType( EnkfVarType.PARAMETER ):
            if not kw in elco_kw:
                node = state[kw]
                init_id = NodeId(0 , geo_id , EnkfStateType.ANALYZED )
                run_id = NodeId(0 , iens , EnkfStateType.ANALYZED )
                node.load( self.init_fs , init_id )
                node.save( self.run_fs , run_id )
            
        for kw_arg in kw_list:
            kw = str(kw_arg[0])
            data = kw_arg[1:]
            self.logger.debug("ADD_SIMULATION %s : %s" % (kw , data))
        
            node = state[kw]
            gen_kw = node.asGenKw()
            gen_kw.setValues(data)
        
            run_id = NodeId(0 , iens , EnkfStateType.ANALYZED )
            node.save( self.run_fs , run_id )
            
        self.run_fs.fsync()
        state_map = self.run_fs.getStateMap()
        state_map[iens] = RealizationStateEnum.STATE_INITIALIZED
        
        self.run_context.startSimulation( iens )
        return self.handleSTATUS([])

    def handleTIMESTEP(self, args):
        enkf_fs_manager = self.ert_handle.getEnkfFsManager()
        enkf_fs = enkf_fs_manager.getCurrentFileSystem()
        time_map = enkf_fs.getTimeMap()
        time_steps = [ ts.datetime().strftime(ErtServer.DATE_FORMAT) for ts in time_map ]

        return self.SUCCESS(time_steps)
