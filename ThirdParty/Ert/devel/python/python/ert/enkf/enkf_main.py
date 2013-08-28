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
from    ert.enkf.enkf_enum        import *
from    ert.ert.enums             import *
from    ert.enkf.ens_config       import *
from    ert.enkf.ecl_config       import *
from    ert.enkf.analysis_config  import *
from    ert.enkf.local_config     import *
from    ert.enkf.model_config     import *
from    ert.enkf.enkf_config_node import *
from    ert.enkf.gen_kw_config    import *
from    ert.enkf.gen_data_config  import *
from    ert.enkf.field_config     import *
from    ert.enkf.enkf_obs         import *
from    ert.enkf.plot_conf        import *
from    ert.enkf.site_config      import *
from    ert.enkf.libenkf          import *
from    ert.enkf.enkf_fs          import *
from    ert.enkf.ert_templates    import *
from    ert.enkf.enkf_state       import *
from ert.util import Log


class EnKFMain(CClass):
    
        
    def __init(self):
        pass


    @classmethod
    def bootstrap(cls , model_config , site_config , strict = True):
        obj = EnKFMain()
        obj.c_ptr = cfunc.bootstrap( site_config , model_config , strict , False )
        return obj
    
    def set_eclbase(self, eclbase):
        cfunc.set_eclbase(self, eclbase)
        
    def __del__(self):
        if self.c_ptr:
            cfunc.free( self )

    #################################################################

    @property
    def ens_size( self ):
        return cfunc.ens_size( self )
    
    @property        
    def ensemble_config(self):
        config = EnsConfig( c_ptr = cfunc.get_ens_config( self ) , parent = self)
        return config
    
    @property 
    def analysis_config(self):
        anal_config = AnalysisConfig( c_ptr = cfunc.get_analysis_config( self ), parent = self)
        return anal_config
    
    @property     
    def model_config(self):
        mod_config = ModelConfig( c_ptr = cfunc.get_model_config( self ) , parent = self)
        return mod_config

    @property     
    def logh(self):
        mog = Log( c_ptr = cfunc.get_logh( self ), parent = self)
        return mog
    
    @property     
    def local_config(self):
        loc_config = LocalConfig( c_ptr = cfunc.get_local_config( self ), parent = self)
        return loc_config
    
    @property     
    def site_config(self):
        site_conf = SiteConfig( c_ptr = cfunc.get_site_config( self ) , parent = self)
        return site_conf
    
    @property     
    def ecl_config(self):
        ecl_conf = EclConfig( c_ptr = cfunc.get_ecl_config( self ) , parent = self)
        return ecl_conf
     
    @property     
    def plot_config(self):
        plot_conf = PlotConf( c_ptr = cfunc.get_plot_config( self ), parent = self)
        return plot_conf
     
    def set_eclbase(self, eclbase):
        cfunc.set_eclbase(self, eclbase)

    def set_datafile(self, datafile):
        cfunc.set_eclbase(self, datafile)

    @property
    def get_schedule_prediction_file(self):
        schedule_prediction_file = cfunc.get_schedule_prediction_file(self)
        return schedule_prediction_file

    def set_schedule_prediction_file(self,file):
        cfunc.set_schedule_prediction_file(self,file)

    @property
    def get_data_kw(self):
        data_kw = cfunc.get_data_kw(self)
        return data_kw

    def clear_data_kw(self):
        cfunc.set_data_kw(self)

    def add_data_kw(self, key, value):
        cfunc.add_data_kw(self, key, value)

    def resize_ensemble(self, value):
        cfunc.resize_ensemble(self, value)

    def del_node(self, key):
        cfunc.del_node(self, key)
        
    @property
    def get_obs(self):
        ob = EnkfObs( c_ptr = cfunc.get_obs( self ), parent = self)
        return ob
    
    def load_obs(self, obs_config_file):
        cfunc.load_obs(self, obs_config_file)
        
    def reload_obs(self):
        cfunc.reload_obs(self)
        
    def set_case_table(self, case_table_file):
        cfunc.set_case_table(self, case_table_file)
        
    @property
    def get_pre_clear_runpath(self):
        pre_clear = cfunc.get_pre_clear_runpath(self)
        return pre_clear
        
    def set_pre_clear_runpath(self, value):
        cfunc.set_pre_clear_runpath(self, value)
        
    def iget_keep_runpath(self, iens):
        ikeep = cfunc.iget_keep_runpath(self, iens)
        return ikeep
        
    def iset_keep_runpath(self, iens, keep_runpath):
        cfunc.iset_keep_runpath(self, iens, keep_runpath)

    @property
    def get_templates(self):
        temp = ErtTemplates( c_ptr = cfunc.get_templates( self ), parent = self)
        return temp
        
    @property
    def get_site_config_file(self):
        site_conf_file = cfunc.get_site_config_file(self)
        return site_conf_file
       
    def initialize_from_scratch(self, parameter_list, iens1, iens2, force_init = True):
        cfunc.initialize_from_scratch(self, parameter_list, iens1, iens2, force_init)

    @property
    def get_fs(self):
        return EnkfFs(c_ptr = cfunc.get_fs(self), parent = self)

    @property
    def get_history_length(self):
        return cfunc.get_history_length(self)
       
    def initialize_from_existing__(self, source_case,source_report_step, source_state, member_mask, ranking_key, node_list):
        cfunc.initialize_from_existing__(self, source_case, source_report_step, source_state, member_mask, ranking_key, node_list)

       
    def copy_ensemble(self, source_case, source_report_step, source_state, target_case, target_report_step, target_state, member_mask, ranking_key, node_list):
        cfunc.copy_ensemble(self, source_case, source_report_step, source_state, target_case, target_report_step, target_state, member_mask, ranking_key, node_list)


    def iget_state(self, ens_memb):
        i_enkf_state = EnKFState( c_ptr = cfunc.iget_state( self ,ens_memb), parent = self)
        return i_enkf_state
    
    def get_observations(self, user_key, obs_count, obs_x, obs_y, obs_std):
        cfunc.get_observations(self, user_key, obs_count, obs_x, obs_y, obs_std)

    def get_observation_count(self, user_key):
        return cfunc.get_observation_count(self, user_key)
 
    @property
    def is_initialized(self):
        return cfunc.is_initialized(self)

    def run(self, boolPtr, init_step_parameter, simFrom, state, mode):
        #{"ENKF_ASSIMILATION" : 1, "ENSEMBLE_EXPERIMENT" : 2, "ENSEMBLE_PREDICTION" : 3, "INIT_ONLY" : 4, "SMOOTHER" : 5}
        if mode == 1:
            cfunc.run_assimilation(self, boolPtr, init_step_parameter, simFrom, state)
        if mode == 2:
            cfunc.run_exp(self, boolPtr, True, init_step_parameter, simFrom, state)
        if mode == 4:
            cfunc.run_exp(self, boolPtr, False, init_step_parameter, simFrom, state)
        if mode == 5:
            cfunc.run_smoother(self, "AUTOSMOOTHER", True)

    @property
    def alloc_caselist(self):
        return StringList(c_ptr = cfunc.alloc_caselist(self), parent = self)

    @property
    def get_current_fs(self):
        return cfunc.get_current_fs(self)

    def user_select_fs(self, input_case):
        cfunc.user_select_fs(self, input_case)

    @property
    def get_alt_fs(self, fs, read_only, create):
        return EnkfFS(cfunc.get_alt_fs(self, fs, read_only, create), parent = self)

    @staticmethod
    def create_new_config(config_file, storage_path, case_name, dbase_type, num_realizations):
        cfunc.create_new_config(config_file, storage_path, case_name, dbase_type, num_realizations)

    def fprintf_config(self):
        cfunc.fprintf_config(self)

    def select_fs(self,path):
        cfunc.select_fs(self,path)
        
    def fs_exists(self,case):
        return cfunc.fs_exists(self,case)
    
##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "enkf_main" , EnKFMain )

cfunc = CWrapperNameSpace("enkf_main")

##################################################################
##################################################################

cfunc.bootstrap                    = cwrapper.prototype("c_void_p enkf_main_bootstrap(char*, char*, bool , bool)")
cfunc.free                         = cwrapper.prototype("void     enkf_main_free( enkf_main )")
cfunc.ens_size                     = cwrapper.prototype("int      enkf_main_get_ensemble_size( enkf_main )")
cfunc.get_ens_config               = cwrapper.prototype("c_void_p enkf_main_get_ensemble_config( enkf_main )")
cfunc.get_model_config             = cwrapper.prototype("c_void_p enkf_main_get_model_config( enkf_main )")
cfunc.get_local_config             = cwrapper.prototype("c_void_p enkf_main_get_local_config( enkf_main )")
cfunc.get_analysis_config          = cwrapper.prototype("c_void_p enkf_main_get_analysis_config( enkf_main)")
cfunc.get_site_config              = cwrapper.prototype("c_void_p enkf_main_get_site_config( enkf_main)")
cfunc.get_ecl_config               = cwrapper.prototype("c_void_p enkf_main_get_ecl_config( enkf_main)")
cfunc.get_plot_config              = cwrapper.prototype("c_void_p enkf_main_get_plot_config( enkf_main)")
cfunc.set_eclbase                  = cwrapper.prototype("void     enkf_main_set_eclbase( enkf_main, char*)")
cfunc.set_datafile                 = cwrapper.prototype("void     enkf_main_set_data_file( enkf_main, char*)")
cfunc.get_schedule_prediction_file = cwrapper.prototype("char* enkf_main_get_schedule_prediction_file( enkf_main )")
cfunc.set_schedule_prediction_file = cwrapper.prototype("void enkf_main_set_schedule_prediction_file( enkf_main , char*)")
cfunc.get_data_kw                  = cwrapper.prototype("c_void_p enkf_main_get_data_kw(enkf_main)")
cfunc.clear_data_kw                = cwrapper.prototype("void enkf_main_clear_data_kw(enkf_main)")
cfunc.add_data_kw                  = cwrapper.prototype("void enkf_main_add_data_kw(enkf_main, char*, char*)")
cfunc.resize_ensemble              = cwrapper.prototype("void enkf_main_resize_ensemble(enkf_main, int)")
cfunc.del_node                     = cwrapper.prototype("void enkf_main_del_node(enkf_main, char*)")
cfunc.get_obs                      = cwrapper.prototype("c_void_p enkf_main_get_obs(enkf_main)")
cfunc.load_obs                     = cwrapper.prototype("void enkf_main_load_obs(enkf_main, char*)")
cfunc.reload_obs                   = cwrapper.prototype("void enkf_main_reload_obs(enkf_main)")
cfunc.set_case_table               = cwrapper.prototype("void enkf_main_set_case_table(enkf_main, char*)")
cfunc.get_pre_clear_runpath        = cwrapper.prototype("bool enkf_main_get_pre_clear_runpath(enkf_main)")
cfunc.set_pre_clear_runpath        = cwrapper.prototype("void enkf_main_set_pre_clear_runpath(enkf_main, bool)")
cfunc.iget_keep_runpath            = cwrapper.prototype("int enkf_main_iget_keep_runpath(enkf_main, int)")
cfunc.iset_keep_runpath            = cwrapper.prototype("void enkf_main_iset_keep_runpath(enkf_main, int, int_vector)")
cfunc.get_templates                = cwrapper.prototype("c_void_p enkf_main_get_templates(enkf_main)")
cfunc.get_site_config_file         = cwrapper.prototype("char* enkf_main_get_site_config_file(enkf_main)")
cfunc.initialize_from_scratch      = cwrapper.prototype("void enkf_main_initialize_from_scratch(enkf_main, stringlist, int, int, bool)")
cfunc.get_fs                       = cwrapper.prototype("c_void_p enkf_main_get_fs(enkf_main)")
cfunc.get_history_length           = cwrapper.prototype("int enkf_main_get_history_length(enkf_main)")
cfunc.initialize_from_existing__   = cwrapper.prototype("void enkf_main_initialize_from_existing__(enkf_main, char*, int, int, bool_vector, char*, stringlist)")
cfunc.copy_ensemble                = cwrapper.prototype("void enkf_main_copy_ensemble(enkf_main, char*, int, int, char*, int, int, bool_vector, char*, stringlist)")
cfunc.get_observations             = cwrapper.prototype("void enkf_main_get_observations(enkf_main, char*, int, long*, double*, double*)") 
cfunc.get_observation_count        = cwrapper.prototype("int enkf_main_get_observation_count(enkf_main, char*)")
cfunc.get_alt_fs                   = cwrapper.prototype("c_void_p enkf_main_get_alt_fs(enkf_main , char* , bool , bool)")
cfunc.is_initialized               = cwrapper.prototype("bool enkf_main_is_initialized(enkf_main)")
cfunc.iget_state                   = cwrapper.prototype("c_void_p enkf_main_iget_state(enkf_main, int)")
cfunc.user_select_fs               = cwrapper.prototype("void enkf_main_user_select_fs(enkf_main , char*)") 
cfunc.get_logh                     = cwrapper.prototype("void enkf_main_get_logh( enkf_main )")
cfunc.run_exp                      = cwrapper.prototype("void enkf_main_run_exp( enkf_main, bool_vector, bool, int, int, int)")
cfunc.run_assimilation             = cwrapper.prototype("void enkf_main_run_assimilation( enkf_main, bool_vector, int, int, int)")
cfunc.run_smoother                 = cwrapper.prototype("void enkf_main_run_smoother(enkf_main, char*, bool)")
cfunc.get_current_fs               = cwrapper.prototype("char* enkf_main_get_current_fs(enkf_main)")
cfunc.alloc_caselist               = cwrapper.prototype("c_void_p enkf_main_alloc_caselist(enkf_main)")
cfunc.fprintf_config               = cwrapper.prototype("void enkf_main_fprintf_config(enkf_main)")
cfunc.create_new_config            = cwrapper.prototype("void enkf_main_create_new_config(char* , char*, char* , char* , int)")
cfunc.select_fs                    = cwrapper.prototype("void enkf_main_select_fs(enkf_main, char*)")
cfunc.fs_exists                    = cwrapper.prototype("bool enkf_main_fs_exists(enkf_main, char*)")
