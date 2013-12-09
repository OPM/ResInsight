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
from ert.cwrap import BaseCClass, CWrapper

from ert.enkf import AnalysisConfig, EclConfig, EnkfObs, EnKFState, LocalConfig, ModelConfig, EnsConfig, PlotConfig, SiteConfig, ENKF_LIB, EnkfSimulationRunner, EnkfFsManager, ErtWorkflowList
from ert.util import SubstitutionList, Log


class EnKFMain(BaseCClass):
    def __init__(self, model_config, site_config, strict=True):
        c_ptr = EnKFMain.cNamespace().bootstrap(site_config, model_config, strict, False)
        super(EnKFMain, self).__init__(c_ptr)

        self.__simulation_runner = EnkfSimulationRunner(self)
        self.__fs_manager = EnkfFsManager(self)


    def set_eclbase(self, eclbase):
        EnKFMain.cNamespace().set_eclbase(self, eclbase)

    def free(self):
        EnKFMain.cNamespace().free(self)

    def getEnsembleSize( self ):
        """ @rtype: int """
        return EnKFMain.cNamespace().get_ensemble_size(self)

    def resizeEnsemble(self, value):
        EnKFMain.cNamespace().resize_ensemble(self, value)

    def ensembleConfig(self):
        """ @rtype: EnsConfig """
        return EnKFMain.cNamespace().get_ens_config(self).setParent(self)

    def analysisConfig(self):
        """ @rtype: AnalysisConfig """
        return EnKFMain.cNamespace().get_analysis_config(self).setParent(self)

    def getModelConfig(self):
        """ @rtype: ModelConfig """
        return EnKFMain.cNamespace().get_model_config(self).setParent(self)

    def logh(self):
        """ @rtype: Log """
        return EnKFMain.cNamespace().get_logh(self).setParent(self)

    def local_config(self):
        """ @rtype: LocalConfig """
        return EnKFMain.cNamespace().get_local_config(self).setParent(self)

    def siteConfig(self):
        """ @rtype: SiteConfig """
        return EnKFMain.cNamespace().get_site_config(self).setParent(self)

    def eclConfig(self):
        """ @rtype: EclConfig """
        return EnKFMain.cNamespace().get_ecl_config(self).setParent(self)

    def plot_config(self):
        """ @rtype: PlotConfig """
        return EnKFMain.cNamespace().get_plot_config(self).setParent(self)

    def set_eclbase(self, eclbase):
        EnKFMain.cNamespace().set_eclbase(self, eclbase)

    def set_datafile(self, datafile):
        EnKFMain.cNamespace().set_datafile(self, datafile)

    def get_schedule_prediction_file(self):
        schedule_prediction_file = EnKFMain.cNamespace().get_schedule_prediction_file(self)
        return schedule_prediction_file

    def set_schedule_prediction_file(self, file):
        EnKFMain.cNamespace().set_schedule_prediction_file(self, file)

    def getDataKW(self):
        """ @rtype: SubstitutionList """
        return EnKFMain.cNamespace().get_data_kw(self)

    def clearDataKW(self):
        EnKFMain.cNamespace().clear_data_kw(self)

    def addDataKW(self, key, value):
        EnKFMain.cNamespace().add_data_kw(self, key, value)




    def del_node(self, key):
        EnKFMain.cNamespace().del_node(self, key)

    def getObservations(self):
        """ @rtype: EnkfObs """
        return EnKFMain.cNamespace().get_obs(self).setParent(self)

    def load_obs(self, obs_config_file):
        EnKFMain.cNamespace().load_obs(self, obs_config_file)

    def reload_obs(self):
        EnKFMain.cNamespace().reload_obs(self)


    def get_pre_clear_runpath(self):
        pre_clear = EnKFMain.cNamespace().get_pre_clear_runpath(self)
        return pre_clear

    def set_pre_clear_runpath(self, value):
        EnKFMain.cNamespace().set_pre_clear_runpath(self, value)

    def iget_keep_runpath(self, iens):
        ikeep = EnKFMain.cNamespace().iget_keep_runpath(self, iens)
        return ikeep

    def iset_keep_runpath(self, iens, keep_runpath):
        EnKFMain.cNamespace().iset_keep_runpath(self, iens, keep_runpath)

    def get_templates(self):
        return EnKFMain.cNamespace().get_templates(self).setParent(self)

    def get_site_config_file(self):
        site_conf_file = EnKFMain.cNamespace().get_site_config_file(self)
        return site_conf_file


    def getHistoryLength(self):
        return EnKFMain.cNamespace().get_history_length(self)


    def copy_ensemble(self, source_case, source_report_step, source_state, target_case, target_report_step,
                      target_state, member_mask, ranking_key, node_list):
        EnKFMain.cNamespace().copy_ensemble(self, source_case, source_report_step, source_state, target_case, target_report_step,
                            target_state, member_mask, ranking_key, node_list)


    def getMemberRunningState(self, ensemble_member):
        """ @rtype: EnKFState """
        return EnKFMain.cNamespace().iget_state(self, ensemble_member).setParent(self)

    def get_observations(self, user_key, obs_count, obs_x, obs_y, obs_std):
        EnKFMain.cNamespace().get_observations(self, user_key, obs_count, obs_x, obs_y, obs_std)

    def get_observation_count(self, user_key):
        return EnKFMain.cNamespace().get_observation_count(self, user_key)


    def createNewConfig(self, storage_path, case_name, dbase_type, num_realizations):
        EnKFMain.cNamespace().create_new_config(self, storage_path, case_name, dbase_type, num_realizations)


    def getEnkfSimulationRunner(self):
        """ @rtype: EnkfSimulationRunner """
        return self.__simulation_runner

    def getEnkfFsManager(self):
        """ @rtype: EnkfFsManager """
        return self.__fs_manager

    def getWorkflowList(self):
        """ @rtype: ErtWorkflowList """
        return EnKFMain.cNamespace().get_workflow_list(self).setParent(self)

##################################################################

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("enkf_main", EnKFMain)


EnKFMain.cNamespace().bootstrap = cwrapper.prototype("c_void_p enkf_main_bootstrap(char*, char*, bool, bool)")
EnKFMain.cNamespace().free = cwrapper.prototype("void enkf_main_free(enkf_main)")

EnKFMain.cNamespace().get_ensemble_size = cwrapper.prototype("int enkf_main_get_ensemble_size( enkf_main )")
EnKFMain.cNamespace().get_ens_config = cwrapper.prototype("ens_config_ref enkf_main_get_ensemble_config( enkf_main )")
EnKFMain.cNamespace().get_model_config = cwrapper.prototype("model_config_ref enkf_main_get_model_config( enkf_main )")
EnKFMain.cNamespace().get_local_config = cwrapper.prototype("local_config_ref enkf_main_get_local_config( enkf_main )")
EnKFMain.cNamespace().get_analysis_config = cwrapper.prototype("analysis_config_ref enkf_main_get_analysis_config( enkf_main)")
EnKFMain.cNamespace().get_site_config = cwrapper.prototype("site_config_ref enkf_main_get_site_config( enkf_main)")
EnKFMain.cNamespace().get_ecl_config = cwrapper.prototype("ecl_config_ref enkf_main_get_ecl_config( enkf_main)")
EnKFMain.cNamespace().get_plot_config = cwrapper.prototype("plot_config_ref enkf_main_get_plot_config( enkf_main)")
EnKFMain.cNamespace().set_eclbase = cwrapper.prototype("ui_return_obj enkf_main_set_eclbase( enkf_main, char*)")
EnKFMain.cNamespace().set_datafile = cwrapper.prototype("void enkf_main_set_data_file( enkf_main, char*)")
EnKFMain.cNamespace().get_schedule_prediction_file = cwrapper.prototype("char* enkf_main_get_schedule_prediction_file( enkf_main )")
EnKFMain.cNamespace().set_schedule_prediction_file = cwrapper.prototype("void enkf_main_set_schedule_prediction_file( enkf_main , char*)")

EnKFMain.cNamespace().get_data_kw = cwrapper.prototype("subst_list_ref enkf_main_get_data_kw(enkf_main)")
EnKFMain.cNamespace().clear_data_kw = cwrapper.prototype("void enkf_main_clear_data_kw(enkf_main)")
EnKFMain.cNamespace().add_data_kw = cwrapper.prototype("void enkf_main_add_data_kw(enkf_main, char*, char*)")

EnKFMain.cNamespace().resize_ensemble = cwrapper.prototype("void enkf_main_resize_ensemble(enkf_main, int)")
EnKFMain.cNamespace().del_node = cwrapper.prototype("void enkf_main_del_node(enkf_main, char*)")
EnKFMain.cNamespace().get_obs = cwrapper.prototype("enkf_obs_ref enkf_main_get_obs(enkf_main)")
EnKFMain.cNamespace().load_obs = cwrapper.prototype("void enkf_main_load_obs(enkf_main, char*)")
EnKFMain.cNamespace().reload_obs = cwrapper.prototype("void enkf_main_reload_obs(enkf_main)")

EnKFMain.cNamespace().get_pre_clear_runpath = cwrapper.prototype("bool enkf_main_get_pre_clear_runpath(enkf_main)")
EnKFMain.cNamespace().set_pre_clear_runpath = cwrapper.prototype("void enkf_main_set_pre_clear_runpath(enkf_main, bool)")
EnKFMain.cNamespace().iget_keep_runpath = cwrapper.prototype("int enkf_main_iget_keep_runpath(enkf_main, int)")
EnKFMain.cNamespace().iset_keep_runpath = cwrapper.prototype("void enkf_main_iset_keep_runpath(enkf_main, int, int_vector)")
EnKFMain.cNamespace().get_templates = cwrapper.prototype("ert_templates_ref enkf_main_get_templates(enkf_main)")
EnKFMain.cNamespace().get_site_config_file = cwrapper.prototype("char* enkf_main_get_site_config_file(enkf_main)")
EnKFMain.cNamespace().get_history_length = cwrapper.prototype("int enkf_main_get_history_length(enkf_main)")

EnKFMain.cNamespace().copy_ensemble = cwrapper.prototype("void enkf_main_copy_ensemble(enkf_main, char*, int, int, char*, int, int, bool_vector, char*, stringlist)")
EnKFMain.cNamespace().get_observations = cwrapper.prototype("void enkf_main_get_observations(enkf_main, char*, int, long*, double*, double*)")
EnKFMain.cNamespace().get_observation_count = cwrapper.prototype("int enkf_main_get_observation_count(enkf_main, char*)")
EnKFMain.cNamespace().iget_state = cwrapper.prototype("enkf_state_ref enkf_main_iget_state(enkf_main, int)")

EnKFMain.cNamespace().get_logh = cwrapper.prototype("log_ref enkf_main_get_logh( enkf_main )")

EnKFMain.cNamespace().get_workflow_list = cwrapper.prototype("ert_workflow_list_ref enkf_main_get_workflow_list(enkf_main)")


EnKFMain.cNamespace().fprintf_config = cwrapper.prototype("void enkf_main_fprintf_config(enkf_main)")
EnKFMain.cNamespace().create_new_config = cwrapper.prototype("void enkf_main_create_new_config(char* , char*, char* , char* , int)")



