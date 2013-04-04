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
import  ecl_config
import  analysis_config
import  local_config
import  model_config
import  enkf_config_node
import  gen_kw_config
import  gen_data_config
import  field_config
import  enkf_obs
import  plot_config
import  site_config
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


cfunc.bootstrap                    = cwrapper.prototype("c_void_p enkf_main_bootstrap(char*, char*, bool)")
cfunc.free                         = cwrapper.prototype("void     enkf_main_free( enkf_main )")
cfunc.run                          = cwrapper.prototype("void     enkf_main_run( enkf_main , int , bool_vector , int , int , int)")
cfunc.ens_size                     = cwrapper.prototype("int      enkf_main_get_ensemble_size( enkf_main )")
cfunc.get_ens_config               = cwrapper.prototype("c_void_p enkf_main_get_ensemble_config( enkf_main )")
cfunc.set_verbose                  = cwrapper.prototype("void     enkf_main_set_verbose( enkf_main , bool )")
cfunc.update                       = cwrapper.prototype("void     enkf_main_UPDATE(enkf_main , int_vector)")
cfunc.get_model_config             = cwrapper.prototype("c_void_p enkf_main_get_model_config( enkf_main )")
cfunc.get_local_config             = cwrapper.prototype("c_void_p enkf_main_get_local_config( enkf_main )")
cfunc.set_eclbase                  = cwrapper.prototype("void     enkf_main_set_eclbase( enkf_main, char*)")
cfunc.set_datafile                 = cwrapper.prototype("void     enkf_main_set_data_file( enkf_main, char*)")
cfunc.get_schedule_prediction_file = cwrapper.prototype("char* enkf_main_get_schedule_prediction_file( enkf_main )")
cfunc.set_schedule_prediction_file = cwrapper.prototype("void enkf_main_set_schedule_prediction_file( enkf_main , char*)")
cfunc.get_data_kw                  = cwrapper.prototype("c_void_p enkf_main_get_data_kw(enkf_main)")
cfunc.clear_data_kw                = cwrapper.prototype("void enkf_main_clear_data_kw(enkf_main)")
cfunc.add_data_kw                  = cwrapper.prototype("void enkf_main_add_data_kw(enkf_main, char*, char*)")
cfunc.get_ensemble_size            = cwrapper.prototype("int enkf_main_get_ensemble_size(enkf_main)")
cfunc.resize_ensemble              = cwrapper.prototype("void enkf_main_resize_ensemble(int)")
cfunc.del_node                     = cwrapper.prototype("void enkf_main_del_node(enkf_main, char*)")
cfunc.get_obs                      = cwrapper.prototype("c_void_p enkf_main_get_obs(enkf_main)")
cfunc.load_obs                     = cwrapper.prototype("void enkf_main_load_obs(enkf_main, char*)")
cfunc.reload_obs                   = cwrapper.prototype("void enkf_main_reload_obs(enkf_main)")
cfunc.set_case_table               = cwrapper.prototype("void enkf_main_set_case_table(enkf_main, char*)")
cfunc.get_pre_clear_runpath        = cwrapper.prototype("bool enkf_main_get_pre_clear_runpath(enkf_main)"),
cfunc.set_pre_clear_runpath        = cwrapper.prototype("void enkf_main_set_pre_clear_runpath(enkf_main, bool)")
cfunc.get_ensemble_size            = cwrapper.prototype("int enkf_main_get_ensemble_size(enkf_main)"),
cfunc.iget_keep_runpath            = cwrapper.prototype("int enkf_main_iget_keep_runpath(enkf_main, int)"),
cfunc.iset_keep_runpath            = cwrapper.prototype("void enkf_main_iset_keep_runpath(enkf_main, int, keep_runpath)")
cfunc.get_templates                = cwrapper.prototype("c_void_p enkf_main_get_templates(enkf_main)")
cfunc.get_site_config_file         = cwrapper.prototype("char* enkf_main_get_site_config_file(enkf_main)")
cfunc.initialize_from_scratch      = cwrapper.prototype("int enkf_main_initialize_from_scratch(enkf_main, stringlist, int, int)")
cfunc.get_ensemble_size            = cwrapper.prototype("int enkf_main_get_ensemble_size(enkf_main)")
cfunc.get_fs                       = cwrapper.prototype("c_void_p enkf_main_get_fs(enkf_main)")
cfunc.get_history_length           = cwrapper.prototype("int enkf_main_get_history_length(enkf_main)")
cfunc.initialize_from_existing__   = cwrapper.prototype("void enkf_main_initialize_from_existing__(enkf_main, char*, int, int, bool_vector, char*, stringlist)")
cfunc.copy_ensemble                = cwrapper.prototype("void enkf_main_copy_ensemble(enkf_main, char*, int, int, char*, int, int, bool_vector, char*, stringlist)")
