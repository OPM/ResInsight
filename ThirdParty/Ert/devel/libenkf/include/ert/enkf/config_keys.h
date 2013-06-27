/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'config_keys.h' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
 */


#ifndef  __CONFIG_KEYS_H__
#define  __CONFIG_KEYS_H__
#ifdef   __cplusplus
extern "C" {
#endif

/* These keys are used as options in KEY:VALUE statements */
#define  FORWARD_INIT_KEY                  "FORWARD_INIT"
#define  MIN_STD_KEY                       "MIN_STD"
#define  INIT_FILES_KEY                    "INIT_FILES"
#define  KEY_KEY                           "KEY"
#define  TEMPLATE_KEY                      "TEMPLATE"
#define  RESULT_FILE_KEY                   "RESULT_FILE"
#define  ECL_FILE_KEY                      "ECL_FILE"
#define  INPUT_FORMAT_KEY                  "INPUT_FORMAT"
#define  OUTPUT_FORMAT_KEY                 "OUTPUT_FORMAT"
#define  OUTPUT_FILE_KEY                   "OUTPUT_FILE"
#define  MAX_KEY                           "MAX"
#define  MIN_KEY                           "MIN"
#define  INPUT_TRANSFORM_KEY               "INPUT_TRANSFORM"
#define  INIT_TRANSFORM_KEY                "INIT_TRANSFORM"
#define  OUTPUT_TRANSFORM_KEY              "OUTPUT_TRANSFORM"
#define  DYNAMIC_KEY                       "DYNAMIC" 
#define  PARAMETER_KEY                     "PARAMETER"
#define  GENERAL_KEY                       "GENERAL"
#define  INCLUDE_KEY                       "INCLUDE"
#define  DEFINE_KEY                        "DEFINE"  
#define  BASE_SURFACE_KEY                  "BASE_SURFACE"

#define  ADD_FIXED_LENGTH_SCHEDULE_KW_KEY  "ADD_FIXED_LENGTH_SCHEDULE_KW"
#define  ANALYSIS_COPY_KEY                 "ANALYSIS_COPY"
#define  ANALYSIS_LOAD_KEY                 "ANALYSIS_LOAD"
#define  ANALYSIS_SET_VAR_KEY              "ANALYSIS_SET_VAR"
#define  ANALYSIS_SELECT_KEY               "ANALYSIS_SELECT"
#define  CASE_TABLE_KEY                    "CASE_TABLE"
#define  CONTAINER_KEY                     "CONTAINER"
#define  DATA_FILE_KEY                     "DATA_FILE"
#define  DATA_KW_KEY                       "DATA_KW"  
#define  DBASE_TYPE_KEY                    "DBASE_TYPE"
#define  DBASE_TYPE_KEY                    "DBASE_TYPE"
#define  DELETE_RUNPATH_KEY                "DELETE_RUNPATH"  
#define  ECLBASE_KEY                       "ECLBASE"
#define  END_DATE_KEY                      "END_DATE"
#define  ENKF_BOOTSTRAP_KEY                "ENKF_BOOTSTRAP"
#define  ENKF_PEN_PRESS_KEY                "ENKF_PEN_PRESS"
#define  ENKF_ALPHA_KEY                    "ENKF_ALPHA"
#define  ENKF_CROSS_VALIDATION_KEY         "ENKF_CROSS_VALIDATION"
#define  ENKF_CV_FOLDS_KEY                 "ENKF_CV_FOLDS"     
#define  ENKF_FORCE_NCOMP_KEY              "ENKF_FORCE_NCOMP"
#define  ENKF_NCOMP_KEY                    "ENKF_NCOMP"     
#define  ENKF_SCALING_KEY                  "ENKF_SCALING"     
#define  ENKF_KERNEL_REG_KEY               "ENKF_KERNEL_REGRESSION"     
#define  ENKF_KERNEL_FUNC_KEY              "ENKF_KERNEL_FUNCTION"     
#define  ENKF_KERNEL_PARAM_KEY             "ENKF_KERNEL_PARAM"     
#define  ENKF_LOCAL_CV_KEY                 "ENKF_LOCAL_CV"     
#define  ENKF_MERGE_OBSERVATIONS_KEY       "ENKF_MERGE_OBSERVATIONS"
#define  ENKF_MODE_KEY                     "ENKF_MODE"
#define  ENKF_RERUN_KEY                    "ENKF_RERUN"
#define  ENKF_SCHED_FILE_KEY               "ENKF_SCHED_FILE"
#define  ENKF_TRUNCATION_KEY               "ENKF_TRUNCATION"
#define  ENSPATH_KEY                       "ENSPATH" 
#define  ITER_CASE_KEY                     "ITER_CASE" 
#define  ITER_COUNT_KEY                    "ITER_COUNT"
#define  FIELD_KEY                         "FIELD"
#define  FORWARD_MODEL_KEY                 "FORWARD_MODEL"
#define  GEN_DATA_KEY                      "GEN_DATA"
#define  GEN_KW_KEY                        "GEN_KW"
#define  GEN_KW_TAG_FORMAT_KEY             "GEN_KW_TAG_FORMAT"
#define  GEN_PARAM_KEY                     "GEN_PARAM"    
#define  GRID_KEY                          "GRID"
#define  HISTORY_SOURCE_KEY                "HISTORY_SOURCE"
#define  HOSY_TYPE_KEY                     "HOST_TYPE"
#define  IGNORE_SCHEDULE_KEY               "IGNORE_SCHEDULE"        
#define  IMAGE_TYPE_KEY                    "IMAGE_TYPE"
#define  IMAGE_VIEWER_KEY                  "IMAGE_VIEWER"      
#define  INIT_SECTION_KEY                  "INIT_SECTION"
#define  INSTALL_JOB_KEY                   "INSTALL_JOB"
#define  JOB_SCRIPT_KEY                    "JOB_SCRIPT"
#define  JOBNAME_KEY                       "JOBNAME"
#define  KEEP_RUNPATH_KEY                  "KEEP_RUNPATH"  
#define  LICENSE_PATH_KEY                  "LICENSE_PATH"
#define  LOAD_SEED_KEY                     "LOAD_SEED"  
#define  LOCAL_CONFIG_KEY                  "LOCAL_CONFIG"
#define  LOG_FILE_KEY                      "LOG_FILE"
#define  LOG_LEVEL_KEY                     "LOG_LEVEL"
#define  LSF_QUEUE_KEY                     "LSF_QUEUE"
#define  LSF_RESOURCES_KEY                 "LSF_RESOURCES"
#define  LSF_SERVER_KEY                    "LSF_SERVER"
#define  TORQUE_QUEUE_KEY                  "TORQUE_QUEUE"
#define  MAX_RESAMPLE_KEY                  "MAX_RESAMPLE"  
#define  MAX_RUNNING_LOCAL_KEY             "MAX_RUNNING_LOCAL"
#define  MAX_RUNNING_LSF_KEY               "MAX_RUNNING_LSF"
#define  MAX_RUNNING_RSH_KEY               "MAX_RUNNING_RSH"
#define  MAX_RUNNING_TORQUE_KEY            "MAX_RUNNING_TORQUE"
#define  MAX_SUBMIT_KEY                    "MAX_SUBMIT" 
#define  NUM_REALIZATIONS_KEY              "NUM_REALIZATIONS"      
#define  OBS_CONFIG_KEY                    "OBS_CONFIG"
#define  OBS_CONFIG_KEY                    "OBS_CONFIG" 
#define  PLOT_DRIVER_KEY                   "PLOT_DRIVER"
#define  PLOT_ERRORBAR_MAX_KEY             "PLOT_ERRORBAR_MAX" 
#define  PLOT_ERRORBAR_KEY                 "PLOT_ERRORBAR"
#define  PLOT_HEIGHT_KEY                   "PLOT_HEIGHT"       
#define  PLOT_PATH_KEY                     "PLOT_PATH"         
#define  PLOT_REFCASE_KEY                  "PLOT_REFCASE"
#define  PLOT_REFCASE_LIST_KEY             "PLOT_REFCASE_LIST"
#define  PLOT_WIDTH_KEY                    "PLOT_WIDTH"        
#define  PRE_CLEAR_RUNPATH_KEY             "PRE_CLEAR_RUNPATH"
#define  QUEUE_SYSTEM_KEY                  "QUEUE_SYSTEM"
#define  QUEUE_OPTION_KEY                  "QUEUE_OPTION" 
#define  QC_PATH_KEY                       "QC_PATH"  
#define  QC_WORKFLOW_KEY                   "QC_WORKFLOW"
#define  REFCASE_KEY                       "REFCASE"
#define  REFCASE_LIST_KEY                  "REFCASE_LIST"
#define  REPORT_CONTEXT_KEY                "REPORT_CONTEXT"
#define  REPORT_SEARCH_PATH_KEY            "REPORT_SEARCH_PATH"
#define  REPORT_LARGE_KEY                  "REPORT_LARGE"   
#define  REPORT_LIST_KEY                   "REPORT_LIST"
#define  REPORT_PATH_KEY                   "REPORT_PATH"
#define  REPORT_WELL_LIST_KEY              "REPORT_WELL_LIST"
#define  REPORT_GROUP_LIST_KEY             "REPORT_GROUP_LIST"
#define  REPORT_TIMEOUT_KEY                "REPORT_TIMEOUT"
#define  RERUN_START_KEY                   "RERUN_START"
#define  RSH_COMMAND_KEY                   "RSH_COMMAND"
#define  RSH_HOST_KEY                      "RSH_HOST"
#define  RUNPATH_KEY                       "RUNPATH"
#define  ITER_RUNPATH_KEY                  "ITER_RUNPATH"
#define  RERUN_PATH_KEY                    "RERUN_PATH"
#define  RUN_TEMPLATE_KEY                  "RUN_TEMPLATE"
#define  RFT_CONFIG_KEY                    "RFT_CONFIG"
#define  RFTPATH_KEY                       "RFTPATH"
#define  SCHEDULE_FILE_KEY                 "SCHEDULE_FILE"
#define  SCHEDULE_PREDICTION_FILE_KEY      "SCHEDULE_PREDICTION_FILE"
#define  SCHEDULE_PREDICTION_FILE_KEY      "SCHEDULE_PREDICTION_FILE"
#define  SCHEDULE_PREDICTION_FILE_KEY      "SCHEDULE_PREDICTION_FILE"
#define  SCHEDULE_PREDICTION_FILE_KEY      "SCHEDULE_PREDICTION_FILE"
#define  SELECT_CASE_KEY                   "SELECT_CASE"
#define  SETENV_KEY                        "SETENV"
#define  STATIC_KW_KEY                     "ADD_STATIC_KW"
#define  STD_CUTOFF_KEY                    "STD_CUTOFF"
#define  SUMMARY_KEY                       "SUMMARY"  
#define  SURFACE_KEY                       "SURFACE"
#define  UPDATE_LOG_PATH_KEY               "UPDATE_LOG_PATH"
#define  UPDATE_PATH_KEY                   "UPDATE_PATH"
#define  UPDATE_RESULTS_KEY                "UPDATE_RESULTS"
#define  SINGLE_NODE_UPDATE_KEY            "SINGLE_NODE_UPDATE"
#define  STORE_SEED_KEY                    "STORE_SEED"
#define  UMASK_KEY                         "UMASK"   
#define  WORKFLOW_JOB_DIRECTORY_KEY        "WORKFLOW_JOB_DIRECTORY"
#define  LOAD_WORKFLOW_KEY                 "LOAD_WORKFLOW"                       
#define  LOAD_WORKFLOW_JOB_KEY             "LOAD_WORKFLOW_JOB"

#define CONFIG_BOOL_STRING( var ) (var) ? "TRUE" : "FALSE"




#ifdef   __cplusplus
}
#endif
#endif
