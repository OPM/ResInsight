#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'ecl_kw.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import  ctypes
from    ert.cwrap.cwrap         import *
from    ert.cwrap.cclass        import CClass
from    ert.util.tvector        import * 
from    ert.job_queue.job_queue import JobQueue
from    enkf_enum               import *
import  ens_config
import  libenkf
import  ert_local

class EnKFMain(CClass):
    
        
    def __init(self):
        pass


    @classmethod
    def bootstrap(cls , model_config , site_config = ert_local.site_config):
        obj = EnKFMain()
        obj.c_ptr = cfunc.bootstrap( site_config , model_config , True , False )
        return obj

        
    def __del__(self):
        if self.c_ptr:
            cfunc.free( self )

    #################################################################

    @property
    def ens_size( self ):
        return cfunc.ens_size( self )
        

    def sim( self ):
        iactive = BoolVector( True )
        iactive[ self.ens_size -1 ] = True
        
        start_state = enkf_state_enum.ANALYZED
        run_mode = enkf_run_enum.ENSEMBLE_EXPERIMENT
        start_report = 0
        init_step_parameters = 0
        cfunc.run( self , run_mode , iactive , init_step_parameters , start_report , start_state )

    def update(self , step_list):
        cfunc.update( self , step_list )
        
        
    @property
    def config(self):
        config = ens_config.EnsConfig( cfunc.get_ens_config( self ))
        return config


##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "enkf_main" , EnKFMain )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("enkf_main")


cfunc.bootstrap       = cwrapper.prototype("c_void_p enkf_main_bootstrap(char*, char*, bool)")
cfunc.free            = cwrapper.prototype("void     enkf_main_free( enkf_main )")
cfunc.run             = cwrapper.prototype("void     enkf_main_run( enkf_main , int , bool_vector , int , int , int)")
cfunc.ens_size        = cwrapper.prototype("int      enkf_main_get_ensemble_size( enkf_main )")
cfunc.get_ens_config  = cwrapper.prototype("c_void_p enkf_main_get_ensemble_config( enkf_main )")
cfunc.set_verbose     = cwrapper.prototype("void     enkf_main_set_verbose( enkf_main , bool )")
cfunc.update          = cwrapper.prototype("void     enkf_main_UPDATE(enkf_main , int_vector)")
